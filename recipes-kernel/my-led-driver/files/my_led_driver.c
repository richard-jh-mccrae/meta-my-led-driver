#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>

static struct gpio_desc *led0 = NULL;
static int led0_state = 0;
static struct kobject *led0_kobj;

static ssize_t my_module_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	/* Write current LED state to user buffer */
	return sprintf(buf, "%d\n", led0_state);
}

static ssize_t my_module_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    // Read input and update LED state, user space can update LED state
	sscanf(buf, "%du", &led0_state);
	if (led0_state == 1 || led0_state == 0) {
		gpiod_set_value(led0, led0_state);
	}
	return count;
}

/* Create the file with name, "led0_state", specify read and write callbacks */
static struct kobj_attribute led_state_attr = __ATTR(led_state, 0660, my_module_show, my_module_store);

static int my_module_probe(struct platform_device *pdev)
{
	int err;
    const char *label;
	char *name;
	char *machine;

	struct device *dev = &pdev->dev;
	struct fwnode_handle *fwnode_btn0;

	printk("LED: Running device probe\n");

	/* Get LED node name, which is machine specific */
#ifdef MACHINE
	if (strcmp(MACHINE, "cyclone5") == 0) {
		strcpy(name, "hps0");
	} else if (strcmp(MACHINE, "beaglebone-yocto") == 0) {
		strcpy(name, "led2");
	} else {
		printk("Unsupported machine type\n");
		return -1;
	}
#else
	printk("Error. Machine type not set\n");
	return -1;
#endif

	fwnode_btn0 = device_get_named_child_node(dev, name);
	if (!fwnode_btn0) {
		printk("Failed to get node name, %s\n", name);
        return -ENODATA;
	}

	// Fetch label
	err = fwnode_property_read_string(fwnode_btn0, "label", &label);
    if (err) {
        printk("Failed to read label property\n");
        return -ENODATA;
    }
    printk("Label is: %s\n", label);

    // Set up LED as GPIO output device
	led0 = devm_fwnode_gpiod_get(dev, fwnode_btn0, NULL, GPIOD_OUT_HIGH, label);
	if(IS_ERR(led0)) {
		printk("Error, could not set up GPIO\n");
		return -EIO;
	}
	printk("LED set up as GPIO device\n");

	/* store current LED state to HIGH */
	led0_state = 1;

    // Create sysfs entry
    led0_kobj = kobject_create_and_add("my_led_sysfs", NULL);
    if(!led0_kobj) {
        printk("Failed to create sysfs entry\n");
        return -ENOMEM;
    }
    // Add attribute
    err = sysfs_create_file(led0_kobj, &led_state_attr.attr);
    if (err) {
        printk("Failed to create sysfs attribute\n");
        // Cleanup sysfs entry
        kobject_put(led0_kobj);
        return -ENOMEM;
    }

    return 0;
}

static int my_module_remove(struct platform_device *pdev)
{
    printk("LED: Running device remove\n");
    // Turn off LED
    gpiod_set_value(led0, 0);
    // Cleanup sysfs entry
    sysfs_remove_file(led0_kobj, &led_state_attr.attr);
    kobject_put(led0_kobj);
    return 0;
}

static struct of_device_id my_driver_of_match[] = {
	{ .compatible = "my-gpio-leds", },
	{ },
};
MODULE_DEVICE_TABLE(of, my_driver_of_match);

static struct platform_driver my_led_driver = {
	.probe = my_module_probe,
	.remove = my_module_remove,
	.driver = {
		.name = "my_led_driver",
		.of_match_table = my_driver_of_match,
	}
};

module_platform_driver(my_led_driver);
MODULE_LICENSE("GPL");
