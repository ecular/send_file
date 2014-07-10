#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <arpa/inet.h>
#include <errno.h>

#define UEVENT_BUFFER_SIZE       2048

GtkWidget* window;

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

int connectsocket(int portnum)
{

    struct sockaddr_in s_add,c_add;
    int sin_size;
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
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

GtkWidget *combo;
char *arg_file;

void showMessageBox()
{
	GtkWidget *dialog = gtk_message_dialog_new((gpointer)window,GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_WARNING,GTK_BUTTONS_OK,"Unallowed Operation");
	gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void on_button_clicked (GtkWidget* button,gpointer data)
{
    /*
     *const gchar *gtk_entry_get_text(GtkEntry *entry)
     *     获得当前文本输入构件的内容
     */
    if ((int)data == 1)
    {
	    printf("arg:%s\n",arg_file);
	    printf("%s\n",gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry)));
	    gtk_widget_destroy(window);
	    return;
    }
    if ((int)data == 2)
    {
        int cfd;
        int recbytes;
        char buffer[1024]={0};   
        char cmd[1024]={0};
        char send1[20]={0},send2[20]={0};
        const gchar* vm_name = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry));
        cfd = connectsocket(8699);
        strcat(send1,"send1 ");
        strcat(send1,vm_name);
        if(-1 == write(cfd,send1,strlen(send1)))
        {
            printf("write fail!\r\n");
            gtk_widget_destroy(window);
            return;
        }
        printf("send to client send1 success!\n");//wait device
        recv (cfd , &buffer, sizeof(buffer), 0); 
        printf("judge result:%s\n",buffer);
        close(cfd);

        if(strcmp(buffer,"NO") == 0)
        {
	    showMessageBox();
            printf("Now Close this Application.\n");
            gtk_widget_destroy(window);
            return;
            printf("Closed!\n");
        }

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
        strcat(cmd,arg_file);
        strcat(cmd," /media/EXCHANGE");
        system(cmd);
	sleep(3);
        system("umount /media/EXCHANGE");
        cfd = connectsocket(8699);
        if(cfd!=-1)
        {
            strcat(send2,"send2 ");
            strcat(send2,vm_name);
            if(-1 == write(cfd,send2,strlen(send2)))
            {
                printf("write fail!\r\n");
                gtk_widget_destroy(window);
                return;
            }
        }
        else
        {
            printf("connect error!");
            gtk_widget_destroy(window);
            return;
        }
        close(cfd);
        gtk_widget_destroy(window);
        return;
    }


}

int main(int argc,char* argv[])
{
    GtkWidget* box;
    GtkWidget* box1;
    GtkWidget* box2;
    GtkWidget* box3;
    GtkWidget* label1;
    GtkWidget* button;
    GtkWidget* sep;
    arg_file = argv[1];

    GtkWidget *vbox;
    GList *items = NULL;
    char rec_buf[200]={0};

    int cfd = connectsocket(8699);
    if(-1 == write(cfd, "*#", 2))
    {
	    printf("write fail!\r\n");
	    gtk_widget_destroy(window);
	    return;
    }

    recv (cfd , &rec_buf, 200, 0); //"vmname1 vmname2 vmname3 "
    printf("start rec:%s\n",rec_buf);
    close(cfd);
    char *p1=rec_buf,*p2;
    while(*p1)
    {
	    p2=strstr(p1," ");
	    *p2=0;
	    items =g_list_append(items,p1);
	    p1=p2+1;

    }
   // items =g_list_append(items,"列表项A");
   // items =g_list_append(items,"列表项B");
   // items =g_list_append(items,"列表项C");
   // items =g_list_append(items,"列表项D");
   // items =g_list_append(items,"列表项E");
    //初始化
    gtk_init(&argc,&argv);
    //设置窗口
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
    gtk_window_set_title(GTK_WINDOW(window),"Send To Other VM");
    gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window),20);

    box = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(window),box);
    box1 = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),box1,FALSE,FALSE,5);
    box2 = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),box2,FALSE,FALSE,5);
    sep = gtk_hseparator_new();//分割线
    gtk_box_pack_start(GTK_BOX(box),sep,FALSE,FALSE,5);
    box3 = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),box3,TRUE,TRUE,5);

    //vbox = gtk_vbox_new(FALSE,0);
    //gtk_box_pack_start(GTK_BOX(box),vbox,TRUE,TRUE,5);
    //combo = gtk_combo_new();
    //gtk_box_pack_start(GTK_BOX(vbox),combo,FALSE,FALSE,5);
    //gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);

    label1 = gtk_label_new("VM Name ：");
    //entry1 = gtk_entry_new();
    combo = gtk_combo_new();
    gtk_box_pack_start(GTK_BOX(box1),label1,FALSE,FALSE,5);
    gtk_box_pack_start(GTK_BOX(box1),combo,FALSE,FALSE,5);
    gtk_combo_set_popdown_strings(GTK_COMBO(combo),items);
    //gtk_box_pack_start(GTK_BOX(box1),entry1,FALSE,FALSE,5);

    button = gtk_button_new_with_label("Send");
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_button_clicked),(gpointer)2);
    gtk_box_pack_start(GTK_BOX(box3),button,TRUE,TRUE,10);
    gtk_widget_show(button);

    button = gtk_button_new_with_label("Cancel");
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_button_clicked),(gpointer)1);
    // g_signal_connect_swapped(G_OBJECT(button),"clicked",G_CALLBACK(gtk_widget_destroy),window);
    gtk_box_pack_start(GTK_BOX(box3),button,TRUE,TRUE,5);

    gtk_widget_show(button);

    gtk_widget_show_all(window);
    gtk_main();
    return FALSE;
}
