#include "bt_handler.h"
#include "defines.h"

#include <stdio.h>
#include <stdlib.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

#define CONNECT_TIMEOUT_SEC 5
#define BUF_SIZE 1
#define IDENT 200

static i32 socket_connect(const char *remote)
{
	struct sockaddr_l2 addr;
	bdaddr_t bdaddr;
	i32 fd, sock_opt;

	fd = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP);
	if (fd < 0) {
		perror("socket");
		goto out;
	}

	/* Bind to local address */
	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, &bdaddr);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		goto error;
	}

	/* Get the socker flags */
	if ((sock_opt = fcntl(fd, F_GETFL, NULL)) < 0) {
		perror("fcntl(F_GETFL)");
		goto error;
	}

	/* Set the socket as non-blocking so we can have a connection timeout */
	if (fcntl(fd, F_SETFL, sock_opt | O_NONBLOCK) < 0) {
		perror("fcntl(F_SETFL)");
		goto error;
	}

	/* Connect to remote device */
	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	str2ba(remote, &addr.l2_bdaddr);

	/* Try connection */
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		if (errno == EINPROGRESS) {
			struct timeval timeout = {
				.tv_sec = CONNECT_TIMEOUT_SEC,
				.tv_usec = 0,
			};

			fd_set wait_set;
            i32 res;

			FD_ZERO(&wait_set);
			FD_SET(fd, &wait_set);

            res = select(fd + 1, NULL, &wait_set, NULL, &timeout);
            if (res < 0) {
                perror("select");
                goto error;
            } else if (res == 0) {
                /* Timeout */
                goto error;
            }
		} else {
			perror("connect");
			goto error;
		}
	}

	/* Reset the socket to blocking once connection is complete */
	if (fcntl(fd, F_SETFL, sock_opt) < 0) {
		perror("fcntl(F_SETFL)");
		goto error;
	}

out:
	return fd;
error:
	if (fd >= 0) {
		close(fd);
	}
	return -1;
}

bool bt_ping(const char *remote)
{
	bool response = false;

	u8 *send_buf;
	u8 *recv_buf;
	i32 fd;

	send_buf = malloc(L2CAP_CMD_HDR_SIZE + BUF_SIZE);
	recv_buf = malloc(L2CAP_CMD_HDR_SIZE + BUF_SIZE);

	if (!send_buf || !recv_buf) {
		perror("malloc");
		goto out;
	}

	fd = socket_connect(remote);
	if (fd < 0) {
		goto out;
	}

	/* Init send buffer */
	for (size_t i = 0; i < BUF_SIZE; ++i) {
		send_buf[L2CAP_CMD_HDR_SIZE + i] = (i % 40) + 'A';
	}

	u8 id = IDENT;

	/* Send command */
	l2cap_cmd_hdr *send_cmd = (l2cap_cmd_hdr *)send_buf;
	l2cap_cmd_hdr *recv_cmd = (l2cap_cmd_hdr *)recv_buf;

	send_cmd->ident = id;
	send_cmd->len = htobs(BUF_SIZE);
	send_cmd->code = L2CAP_ECHO_REQ;

	if (send(fd, send_buf, L2CAP_CMD_HDR_SIZE + BUF_SIZE, 0) <= 0) {
		perror("send");
		goto out;
	}

	/* Wait for response */
	while (1) {
		struct pollfd pf[1];
		int err;

		pf[0].fd = fd;
		pf[0].events = POLLIN;

		if ((err = poll(pf, 1, 3000)) < 0) {
			perror("poll");
			goto out;
		}

		if (!err) {
			printf("No response from device: %s\n", remote);
			break;
		}

		if ((err = recv(fd, recv_buf, L2CAP_CMD_HDR_SIZE + BUF_SIZE,
				0)) < 0) {
			perror("recv");
			goto out;
		}

		if (!err) {
			printf("Disconnected\n");
			break;
		}

		recv_cmd->len = btohs(recv_cmd->len);

		/* Confirm id */
		if (recv_cmd->ident != id) {
			continue;
		}

		/* Check type */
		if (recv_cmd->code == L2CAP_ECHO_RSP) {
			response = true;
			break;
		}

		if (recv_cmd->code == L2CAP_COMMAND_REJ) {
			printf("Peer rejected the echo request\n");
			goto out;
		}
	}

out:
	if (fd >= 0) {
		close(fd);
	}
	if (send_buf)
		free(send_buf);
	if (recv_buf)
		free(recv_buf);

	return response;
}
