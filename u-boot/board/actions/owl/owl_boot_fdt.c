
/*
 * (C) Copyright 2012
 * Actions Semi .Inc
 */
#include <common.h>
#include <linux/ctype.h>
#include <libfdt.h>
#include <fdt_support.h>

#include <asm/arch/owl_afi.h>

 
DECLARE_GLOBAL_DATA_PTR;

#define ENUM_HEADER			1
#define ENUM_TAIL			2
#define ENUM_REMOVE			3

extern int read_mi_item(char *name, void *buf, unsigned int count);

static void cmd_string_remove(char *o, char *s)
{
	char *m, *f, *p;
	unsigned int len;
	char str[64];

	p = s;
	while (p) {
		m = strstr(p, " ");
		if (m)
			len = m - p;
		else
			len = strlen(p);

		strncpy(str, p, len);
		str[len] = 0;

		f = strstr(o, str);
		if (f) {
			len = strlen(f);
			memmove(f, f + strlen(str), len - strlen(str));
			*(f + len - strlen(str)) = 0;
		}
		if (!m)
			break;

		while(m && isspace(*m))
			m++;
		p = m;
	}
}


static void boot_append_remove_args(char *arg, int action)
{
	char *s;
	char cmdline[CONFIG_SYS_BARGSIZE];
	const char *ns = cmdline;

	if (!arg)
		return;

	if ((s = getenv("bootargs")) == NULL)
		s = "";

	if(action == ENUM_HEADER)
		sprintf(cmdline, "%s %s", arg, s);
	else if(action == ENUM_TAIL)
		sprintf(cmdline, "%s %s", s, arg);
	else if(action == ENUM_REMOVE)
		cmd_string_remove(cmdline, arg);
	else
		return;
	debug("cmdline: %s\n", cmdline);

	setenv("bootargs", ns);
}

static int boot_fdt_setprop(void *blob)
{
	int node;
	char new_prop[CONFIG_SYS_BARGSIZE];
    char *s;

	s = getenv("bootargs");
	strcpy(new_prop, s);
	node = fdt_find_or_add_subnode(blob, 0, "chosen");
	if (node < 0){
		printf("error when find chosen node: %d", node);
		return -1;
	}

	fdt_setprop(blob, node, "bootargs", new_prop, strlen(new_prop) + 1);
	return 0;
}

int boot_append_bootargs_add(void *fdt)
{
	int node = 0, ret = 0;
	const char *bootargs;
	char *bootargs_add;
	char new_prop[CONFIG_SYS_BARGSIZE];

	bootargs_add = getenv("bootargs.add");
	if (bootargs_add == NULL)
		return 0;

	/* find "/chosen" node. */
	node = fdt_path_offset(fdt, "/chosen");
	if (node < 0) {
		printf("fdt_path_offset failed %d\n", node);
		return -1;
	}

	bootargs = fdt_getprop(fdt, node, "bootargs", NULL);
	if (!bootargs) {
		printf("%s: Warning: No bootargs in fdt %s\n", __func__,
		       bootargs);
		return -1;
	}

	snprintf(new_prop, CONFIG_SYS_BARGSIZE, "%s %s",
		 bootargs, bootargs_add);

	ret = fdt_setprop(fdt, node, "bootargs",
			  new_prop, strlen(new_prop) + 1);
	if (ret < 0) {
		printf("could not set bootargs %s\n", new_prop);
		return ret;
	}
	return 0;
}

void owl_boot_fdt_setup(void *blob)
{
	char buf[64], sn[32];
	char *s;
	int index;
	
	int boot_dev = owl_get_boot_dev();
	
	if (owl_get_kernel_version() == KERNEL_VERSION3)
	{
		strcpy(buf, "earlyprintk");
	}
	else
	{
		strcpy(buf, "earlycon");
	}
	
	boot_append_remove_args(buf, ENUM_TAIL);
	
	strcpy(buf, "clk_ignore_unused");
	boot_append_remove_args(buf, ENUM_TAIL);
	
	strcpy(buf, "selinux=0");
	boot_append_remove_args(buf, ENUM_TAIL);

	index = owl_afi_get_serial_number();
	
	sprintf(buf, "console=ttyS%d", index);
	boot_append_remove_args(buf, ENUM_HEADER);
	
    switch(boot_dev)
    {
    case OWL_BOOTDEV_NAND:
    	sprintf(buf, "boot_dev=%s", "nand");
		break;
	case OWL_BOOTDEV_SD0:
		sprintf(buf, "boot_dev=%s", "sd0");
		break;		
	case OWL_BOOTDEV_SD1:
		sprintf(buf, "boot_dev=%s", "sd1");
		break;
	case OWL_BOOTDEV_SD2:
		sprintf(buf, "boot_dev=%s", "sd2");
		break;			
	case OWL_BOOTDEV_SD02NAND:
		sprintf(buf, "boot_dev=%s", "sd02nand");
		break;				
	case OWL_BOOTDEV_SD02SD2:
		sprintf(buf, "boot_dev=%s", "sd02sd2");
		break;			
	case OWL_BOOTDEV_NOR:
		sprintf(buf, "boot_dev=%s", "nor");
		break;
    }
    
	boot_append_remove_args(buf, ENUM_TAIL);

    boot_fdt_setprop(blob);
	boot_append_bootargs_add(blob);
	
	printf("cmdline: %s\n", getenv("bootargs"));
}
