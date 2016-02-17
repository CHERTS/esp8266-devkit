OUTPUT_FORMAT("elf32-xtensa-le")


SECTIONS
{
	.irom0.literal : ALIGN(4) SUBALIGN(4) {
		webpages_espfs_start = .;
		*(*)
		webpages_espfs_end = .;
		webpages_espfs_size = webpages_espfs_end - webpages_espfs_start;
	}
}