// SPDX-License-Identifier: GPL-2.0
/*
 * kbled.c — динамически загружаемый модуль для управления светодиодами клавиатуры
 *
 * Использует sysfs-параметр 'state' для установки набора светодиодов:
 *   bit 0 (0x1) — NumLock
 *   bit 1 (0x2) — CapsLock
 *   bit 2 (0x4) — ScrollLock
 */

 #include <linux/module.h>
 #include <linux/init.h>
 #include <linux/kobject.h>
 #include <linux/sysfs.h>
 #include <linux/vt_kern.h>
 #include <linux/console_struct.h>
 #include <linux/uaccess.h>
 
 #define LED_NUM  0x1
 #define LED_CAPS 0x2
 #define LED_SCROLL 0x4
 
 static struct kobject *kbled_kobj;
 static unsigned char state;
 
 static void set_kb_leds(unsigned char leds)
 {
     struct vc_data *vc = &vc_cons[fg_console].d;
     vc->vc_sw->con_set_leds(vc, leds);
 }
 
 static ssize_t state_show(struct kobject *kobj,
                           struct kobj_attribute *attr, char *buf)
 {
     return sprintf(buf, "0x%02x\n", state);
 }
 
 static ssize_t state_store(struct kobject *kobj,
                            struct kobj_attribute *attr,
                            const char *buf, size_t count)
 {
     unsigned long tmp;
     if (kstrtoul(buf, 0, &tmp) < 0)
         return -EINVAL;
 
     state = tmp & (LED_NUM | LED_CAPS | LED_SCROLL);
     set_kb_leds(state);
     return count;
 }
 
 static struct kobj_attribute state_attr =
     __ATTR(state, 0664, state_show, state_store);
 
 static int __init kbled_init(void)
 {
     int ret;
 
     /* Создаём kobject в /sys/kernel/kbled */
     kbled_kobj = kobject_create_and_add("kbled", kernel_kobj);
     if (!kbled_kobj)
         return -ENOMEM;
 
     ret = sysfs_create_file(kbled_kobj, &state_attr.attr);
     if (ret) {
         kobject_put(kbled_kobj);
         return ret;
     }
 
     /* Инициализируем LED в 0 */
     state = 0;
     set_kb_leds(state);
     pr_info("kbled: module loaded, initial state=0x%02x\n", state);
     return 0;
 }
 
 static void __exit kbled_exit(void)
 {
     /* Сброс LED и удаление sysfs */
     state = 0;
     set_kb_leds(state);
     sysfs_remove_file(kbled_kobj, &state_attr.attr);
     kobject_put(kbled_kobj);
     pr_info("kbled: module unloaded, LEDs reset\n");
 }
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("Your Name");
 MODULE_DESCRIPTION("Keyboard LED control via sysfs");
 module_init(kbled_init);
 module_exit(kbled_exit);
// SPDX-License-Identifier: GPL-2.0
/*
 * kbled.c — динамически загружаемый модуль для управления светодиодами клавиатуры
 *
 * Использует sysfs-параметр 'state' для установки набора светодиодов:
 *   bit 0 (0x1) — NumLock
 *   bit 1 (0x2) — CapsLock
 *   bit 2 (0x4) — ScrollLock
 */

 #include <linux/module.h>
 #include <linux/init.h>
 #include <linux/kobject.h>
 #include <linux/sysfs.h>
 #include <linux/vt_kern.h>
 #include <linux/console_struct.h>
 #include <linux/uaccess.h>
 
 #define LED_NUM  0x1
 #define LED_CAPS 0x2
 #define LED_SCROLL 0x4
 
 static struct kobject *kbled_kobj;
 static unsigned char state;
 
 static void set_kb_leds(unsigned char leds)
 {
     struct vc_data *vc = &vc_cons[fg_console].d;
     vc->vc_sw->con_set_leds(vc, leds);
 }
 
 static ssize_t state_show(struct kobject *kobj,
                           struct kobj_attribute *attr, char *buf)
 {
     return sprintf(buf, "0x%02x\n", state);
 }
 
 static ssize_t state_store(struct kobject *kobj,
                            struct kobj_attribute *attr,
                            const char *buf, size_t count)
 {
     unsigned long tmp;
     if (kstrtoul(buf, 0, &tmp) < 0)
         return -EINVAL;
 
     state = tmp & (LED_NUM | LED_CAPS | LED_SCROLL);
     set_kb_leds(state);
     return count;
 }
 
 static struct kobj_attribute state_attr =
     __ATTR(state, 0664, state_show, state_store);
 
 static int __init kbled_init(void)
 {
     int ret;
 
     /* Создаём kobject в /sys/kernel/kbled */
     kbled_kobj = kobject_create_and_add("kbled", kernel_kobj);
     if (!kbled_kobj)
         return -ENOMEM;
 
     ret = sysfs_create_file(kbled_kobj, &state_attr.attr);
     if (ret) {
         kobject_put(kbled_kobj);
         return ret;
     }
 
     /* Инициализируем LED в 0 */
     state = 0;
     set_kb_leds(state);
     pr_info("kbled: module loaded, initial state=0x%02x\n", state);
     return 0;
 }
 
 static void __exit kbled_exit(void)
 {
     /* Сброс LED и удаление sysfs */
     state = 0;
     set_kb_leds(state);
     sysfs_remove_file(kbled_kobj, &state_attr.attr);
     kobject_put(kbled_kobj);
     pr_info("kbled: module unloaded, LEDs reset\n");
 }
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("Your Name");
 MODULE_DESCRIPTION("Keyboard LED control via sysfs");
 module_init(kbled_init);
 module_exit(kbled_exit);
  