/*
 * convert-vpu-firmware - Convert VPU firmware files distributed by Freescale
 *                        into the format used by the CODA kernel driver.
 *
 *  Copyright 2013-2014 Philipp Zabel, Pengutronix
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

struct fw_header_info {
	u_int8_t  platform[12];
	u_int32_t size;
};

void usage() {
	fprintf(stderr, "usage: convert-vpu-firmware <infile> <outfile>\n");
	fprintf(stderr, "example: convert-vpu-firmware vpu_fw_imx53.bin v4l-coda7541-imx53.bin\n");
}

int main(int argc, char *argv[])
{
	int fd, i, n, size;
	struct fw_header_info header;
	u_int32_t *buf;

	if (argc < 3) {
		usage();
		return 0;
	}

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open source file '%s'.\n", argv[1]);
		return EXIT_FAILURE;
	}

	n = read(fd, &header, sizeof(header));
	if (n != sizeof(header)) {
		fprintf(stderr, "Failed to read header.\n");
		return EXIT_FAILURE;
	}

	size = header.size * sizeof(u_int16_t);
	buf = malloc(size);
	if (buf == NULL)
		return EXIT_FAILURE;

	n = read(fd, buf, size);
	if (n != size) {
		fprintf(stderr, "Failed to read firmware.\n");
		return EXIT_FAILURE;
	}

	close(fd);

	for (i = 0; i < size / 4; i += 2) {
		u_int32_t tmp = buf[i + 1] << 16 | buf[i + 1] >> 16;
		buf[i + 1] = buf[i] << 16 | buf[i] >> 16;
		buf[i] = tmp;
	}

	fd = open(argv[2], O_RDWR | O_CREAT, 0644);
	if (fd < 0)
		error("Failed to open target file '%s'.\n", argv[2]);

	n = write(fd, buf, size);
	if (n != size)
		error("Failed to write reordered firmware\n");

	close(fd);

	return 0;
}
