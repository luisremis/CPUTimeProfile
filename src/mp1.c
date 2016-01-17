#define LINUX

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h> /* Necessary because we use the proc fs */
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>

#include "mp1_given.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group_30");
MODULE_DESCRIPTION("CS-423 MP1");

#define DEBUG 1

#define procfs_name "status"

/* Allocated buffer for pid reception, which is an int32 */
#define INCOMMING_MAX_SIZE  4

#define TIMER_SET     0
#define TIMER_NOT_SET 1

/* Lock data structure */

DEFINE_SPINLOCK( mr_lock );

/* Timer data structures */
static struct timer_list timer_5sec;
unsigned int timer_set;

/* proc filesystem and linked list data structures */

LIST_HEAD(mp1_linked_list) ;

struct user_time_pid {
     unsigned long pid;
     unsigned long user_time;
     struct list_head mp1_list ;
} ;

//--------------

/* Methods */ 

static struct workqueue_struct *my_wq;

struct mp1_work{
  struct work_struct my_work;
} ;

struct mp1_work *work;


static void my_wq_function( struct work_struct *work)
{
    int ret;
    struct list_head *pos, *q;
    struct user_time_pid *tmp;

    printk( "Running Bottom-Half work \n");

    // should put a lock here, because the registration can cause inconsistent state.
    spin_lock(&mr_lock);

    list_for_each_safe(pos, q, & mp1_linked_list){
     tmp= list_entry(pos, struct user_time_pid, mp1_list);
     ret = get_cpu_use(tmp->pid, &tmp->user_time) ; 
      if ( ret ){
        printk( "Process %ld does not exist anymore, will be removed\n", tmp->pid);
        list_del(pos);
        kfree(tmp);
      }
    }

    spin_unlock(&mr_lock);

    return;
}

void timer_5sec_callback( unsigned long data )
{
    // We only trigger the work when the list is not empty
    if ( 0 == list_empty( &mp1_linked_list) )
    {
      queue_work( my_wq, (struct work_struct *)work );
      
      //re-shoot the timer
      //This can be here or at the buttom half. 
      //We put it here to avoid jitter in the 5seconds call.
      mod_timer( &timer_5sec, jiffies + msecs_to_jiffies(5000) );
    }
    else{
      //printk( "No process register, no work to be set \n" );
      timer_set = TIMER_NOT_SET;
    }
      
}

static int status_proc_show(struct seq_file *m, void *v) {

  struct user_time_pid  *tmp = NULL ; 
  // should put a lock here, because the registration can cause inconsisten state.
  spin_lock(&mr_lock);

  //We used seq_file to print because it is a tool that allow us no to
  //worry about paging. 
  list_for_each_entry ( tmp , & mp1_linked_list, mp1_list ) 
  { 
    seq_printf(m, "PID: %ld - CPU_time: %ld \n" , tmp->pid, tmp->user_time ); 
  }

  spin_unlock(&mr_lock);

  return 0;
}

static int status_proc_open(struct inode *inode, struct  file *file) {

   printk("calling status_proc_open \n" );
   return single_open(file, status_proc_show, NULL);
}

int procfile_write(struct file *file, const char *buffer, 
                      unsigned long count, void *data)
{
    int ret;
    static unsigned long buffer_size = 0;
    struct user_time_pid* new_process;
    new_process = kmalloc(sizeof(struct user_time_pid), GFP_KERNEL); 
    new_process->pid = 0 ; // Inicially with any value, does not matter

    /* get buffer size */
    buffer_size = count;
    if (buffer_size > INCOMMING_MAX_SIZE ) {
      buffer_size = INCOMMING_MAX_SIZE;
    }

    /* write data to the buffer */
    if ( copy_from_user( (void *)&(new_process->pid), buffer, buffer_size) ) {
      return -EFAULT;
    }

    ret = get_cpu_use(new_process->pid, &new_process->user_time) ; 
    if ( ret ){
      printk( "Process received %ld does not exist \n", new_process->pid);
      kfree((void *) new_process);
      return -1;
    }
    else{
          // should put a lock here, conflict with other methods using the list.
          spin_lock(&mr_lock);
          list_add ( &new_process->mp1_list , &mp1_linked_list ) ;
          spin_unlock(&mr_lock);

          printk("receive: %ld \n", new_process->pid);

          // If timer not set, we set the timer
          // This means this is the only process in the list
          if (timer_set == TIMER_NOT_SET){
            ret = mod_timer( &timer_5sec, jiffies + msecs_to_jiffies(5000) );
            timer_set = TIMER_SET;
          }
    }

    return buffer_size;
}

static const struct file_operations status_proc_fops = {
  .owner = THIS_MODULE,
  .open = status_proc_open,
  .read = seq_read, // This simplifies the reading process
  .write = procfile_write, // Write our own writing method
  .llseek = seq_lseek,
  .release = single_release,
};

// mp1_init - Called when module is loaded
int __init mp1_init(void)
{
    struct proc_dir_entry *proc_parent; 
    struct proc_dir_entry *proc_write_entry;

    #ifdef DEBUG
    printk(KERN_ALERT "MP1 MODULE LOADING\n");
    #endif
    // Insert your code here ...

    /* Creation proc filesytems dir and file */
    proc_parent = proc_mkdir("mp1",NULL);
    proc_write_entry =proc_create("status", 0666, proc_parent, &status_proc_fops);

    /* Timer inicialization */
    setup_timer( &timer_5sec, timer_5sec_callback, 0 );

    // The timer will be set only when at least one process
    // is registered. 
    timer_set = TIMER_NOT_SET;

    /* Working queue init */

    my_wq = create_workqueue("my_queue");
    if (my_wq) {

      /* Queue some work (item 1) */
      work = (struct mp1_work *)kmalloc(sizeof(struct mp1_work), GFP_KERNEL);
      if (work) {

         INIT_WORK( (struct work_struct *)work, my_wq_function );

      }
    }

    printk(KERN_ALERT "MP1 MODULE LOADED\n");
    return 0;   
}

// mp1_exit - Called when module is unloaded
void __exit mp1_exit(void)
{
    int ret;
    struct list_head *pos, *q;
    struct user_time_pid *tmp;

    #ifdef DEBUG
    printk(KERN_ALERT "MP1 MODULE UNLOADING\n");
    #endif
    // Insert your code here ...

    /* Deleting timer */ 
    ret = del_timer( &timer_5sec );
    if (ret) printk("Deleting an active timer\n");

    /* Deleting working queue */

    kfree( (void *)work );   
    flush_workqueue( my_wq );
    destroy_workqueue( my_wq );

    /* Deleting list */

    /* let'sfree the user_time_pid items. since we will be removing items
    * off the list using list_del() we need to use a safer version of the list_for_each() 
    * macro aptly named list_for_each_safe(). Note that you MUST use this macro if the loop 
    * involves deletions of items (or moving items from one list to another).
    */
    //printk("deleting the list using list_for_each_safe()\n");
    list_for_each_safe(pos, q, & mp1_linked_list){
     tmp= list_entry(pos, struct user_time_pid, mp1_list);
     printk("freeing item pid= %ld \n", tmp->pid);
     list_del(pos);
     kfree(tmp);
    }

    /* Removing entrys in proc filesystem */
    
    remove_proc_entry("mp1/status",NULL);
    remove_proc_entry("mp1",NULL);

    printk(KERN_ALERT "MP1 MODULE UNLOADED\n");
}

// Register init and exit funtions
module_init(mp1_init);
module_exit(mp1_exit);
