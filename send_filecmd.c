/*a.out filename vmname*/
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>

#define UEVENT_BUFFER_SIZE       2048
//int cal_num(char *str)
//{
//    int sum = 0;
//    char *end = str+strlen(str);
//    char *start = str;
//    while(start<=end)
//        sum+=*start++;
//    return sum%8090+(65535-8090);
//}

static int init_hotplug_sock ( void )
{
	struct sockaddr_nl snl ;
	const int buffersize = 16 * 1024 * 1024;
	int retval ;

	memset (& snl , 0x00, sizeof ( struct sockaddr_nl));
	snl .nl_family = AF_NETLINK;
	snl .nl_pid = getpid ();
	snl .nl_groups = 1;

	int hotplug_sock = socket (PF_NETLINK, SOCK_DGRAM , NETLINK_KOBJECT_UEVENT);
	if ( hotplug_sock == -1) {
		printf ( "error getting socket: %s" , strerror ( errno ));
		return -1;
	}

	/* set receive buffersize */
	setsockopt ( hotplug_sock , SOL_SOCKET , SO_RCVBUFFORCE, & buffersize , sizeof ( buffersize ));

	retval = bind ( hotplug_sock , ( struct sockaddr *) & snl , sizeof ( struct sockaddr_nl));
	if (retval < 0) {
		printf ( "bind failed: %s" , strerror ( errno ));
		close ( hotplug_sock );
		hotplug_sock = -1;
		return -1;
	}

	return hotplug_sock ;
}

int connectsocket()
{

	struct sockaddr_in s_add,c_add;
	int sin_size;
	int cfd = socket(AF_INET, SOCK_STREAM, 0);
	int portnum = 8699; 
	if(-1 == cfd)
	{
		printf("socket fail ! \r\n");
		return -1;
	}

	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr= inet_addr("127.0.0.1");
	s_add.sin_port=htons(portnum);

	if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
	{
		printf("connect fail !\r\n");
		return -1;
	}
	return cfd;
}

int main(int argc, char **argv)
{
	int cfd;
	int recbytes;
	char buffer[1024]={0};   
	char cmd[1024]={0};
	//int portnum = cal_num(argv[1]); 

	cfd = connectsocket();
	if(-1 == write(cfd,"send1 test2",11))
	{
		printf("write fail!\r\n");
		return -1;
	}
	printf("send to client send1 success!\n");//wait device
	recv (cfd , &buffer, sizeof(buffer), 0); 
	printf("judge result:%s\n",buffer);
	close(cfd);

	if(strstr(buffer,"NO"))
		return 0;

	int hotplug_sock  = init_hotplug_sock ();
	while (1)
	{
		char buf [UEVENT_BUFFER_SIZE *2] = {0};
		recv (hotplug_sock , &buf, sizeof (buf), 0); 
		if(sizeof(buf)>0)
			break;
	}
	sleep(5);
	strcat(cmd,"cp ");
	strcat(cmd,argv[1]);
	strcat(cmd," /media/EXCHANGE");
	system(cmd);
	system("umount /media/EXCHANGE");
	cfd = connectsocket();
	if(cfd!=-1)
	{
		if(-1 == write(cfd,"send2 test2",11))
		{
			printf("write fail!\r\n");
			return -1;
		}
		// if(recv(cfd,buffer,1024, 0)!=-1)
		// {
		//         printf("received responds! %s\n",buffer);
		// } 
	}
	else
	{
		printf("connect error!");
		return -1;
	}
	close(cfd);
	return 0;
}
