//
// Created by hhx on 2019/1/10.
//

#ifndef LOGREC_SERVER_H
#define LOGREC_SERVER_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#ifdef _APPLE_
#include <sys/uio.h>
#else
#include <sys/sendfile.h>

#endif

struct FileInfo {
	int fd;
	int nsize;
};

class Server {
public:
	Server() {
		std::ifstream conf("server.conf");
		conf >> ip >> port;

		std::string filename;
		while (conf >> filename) {
			FileInfo info;
			info.fd = open(filename.c_str(), O_RDONLY, 0);
			struct stat st;
			fstat(info.fd, &st);
			info.nsize = st.st_size;
			fds.push_back(info);
		}
		conf.close();
	}
	virtual ~Server() { }

	void Work() {
		struct sockaddr_in addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr=inet_addr(ip.c_str());

		server_fd = socket(PF_INET, SOCK_STREAM, 0);
		int ret = bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
		if (ret != 0) {
			_exit(-1);
		}

		ret = listen(server_fd, 5);
		if (ret != 0) {
			_exit(-1);
		}

		socklen_t len = sizeof(addr);
		client_fd = accept(server_fd, (struct sockaddr*)&addr, &len);
		if (client_fd == 0) {
			_exit(-1);
		}

		SendFile();
	}

private:
	void SendFile() {
		for (auto k : fds) {
#ifdef _APPLE_
			off_t len = k.nsize;
			sendfile(k.fd, client_fd, 0, &len, NULL, 0);
#else
			sendfile(client_fd, k.fd, NULL, k.nsize);
#endif
		}
		close(client_fd);
		close(server_fd);
	}

	std::vector<FileInfo> fds;
	int server_fd;
	int client_fd;
	std::string ip;
	int port;
};


#endif //LOGREC_SERVER_H
