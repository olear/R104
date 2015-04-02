/*
   See Documentation/x86/rdc.txt

   mark@bifferos.com
*/

#include <linux/pci.h>
#include <asm/pci-direct.h>
#include "cpu.h"


static void __cpuinit rdc_identify(struct cpuinfo_x86 *c)
{
	u16 vendor, device;
	u32 customer_id;

	/* RDC CPU is SoC (system-on-chip), Northbridge is always present. */
	vendor = read_pci_config_16(0, 0, 0, PCI_VENDOR_ID);
	device = read_pci_config_16(0, 0, 0, PCI_DEVICE_ID);

	if (vendor != PCI_VENDOR_ID_RDC || device != PCI_DEVICE_ID_RDC_R6020)
		return;  /* not RDC */

	/* NB: We could go on and check other devices, e.g. r6040 NIC, but
	   that's probably overkill */

	strcpy(c->x86_vendor_id, "RDC");
	c->x86_vendor = X86_VENDOR_RDC;

	customer_id = read_pci_config(0, 0, 0, 0x90);

	switch (customer_id) {
		/* id names are from RDC */
	case 0x00321000:
		strcpy(c->x86_model_id, "R3210/R3211");
		break;
	case 0x00321001:
		strcpy(c->x86_model_id, "AMITRISC20000/20010");
		break;
	case 0x00321002:
		strcpy(c->x86_model_id, "R3210X/Edimax");
		break;
	case 0x00321003:
		strcpy(c->x86_model_id, "R3210/Kcodes");
		break;
	case 0x00321004:  /* tested */
		strcpy(c->x86_model_id, "S3282/CodeTek");
		break;
	case 0x00321007:
		strcpy(c->x86_model_id, "R8610");
		break;
	default:
		printk(KERN_INFO "Unrecognised Customer ID (0x%x) please "
			"report to bifferos@yahoo.co.uk\n", customer_id);

		/* We'll default to the R321x since that's mentioned
		   elsewhere in the kernel sources */
		strcpy(c->x86_model_id, "R321x");

		/* blank the vendor_id, so we get a warning that this
		   is unsupported, your system may be unstable etc...
		   Is there a better way? */
		strcpy(c->x86_vendor_id, "");
	}
}

static const struct cpu_dev __cpuinitconst rdc_cpu_dev = {
	.c_vendor	= "RDC",
	.c_ident	= { "RDC" },
	.c_identify	= rdc_identify,
	.c_x86_vendor	= X86_VENDOR_RDC,
};

cpu_dev_register(rdc_cpu_dev);
