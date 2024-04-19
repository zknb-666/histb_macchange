#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
int readbootargs(){
    FILE *fp=NULL;
    FILE *bootargs=NULL;
    char tmp[2];
    system("dd if=/dev/mmcblk0p2 of=/tmp/bootargs.bin count=1024 bs=1024");
    if((fp=fopen("/tmp/bootargs_input.txt","w+"))==NULL||(bootargs=fopen("/tmp/bootargs.bin","r"))==NULL)
        return 0;
    fseek(bootargs,4,SEEK_SET);
    for((fread(tmp+0,1,1,bootargs),fread(tmp+1,1,1,bootargs));tmp[0]||tmp[1];(fread(tmp+0,1,1,bootargs),fread(tmp+1,1,1,bootargs)))
    {   
        if(tmp[0]&&!tmp[1])
        {
            fwrite(tmp+0,1,1,fp);
            fputc('\n',fp);
        }
        else if(!tmp[0]&&tmp[1])
        {
            fputc('\n',fp);
            fwrite(tmp+1,1,1,fp);
        }
        else
        {   
            fwrite(tmp+0,1,1,fp);
            fwrite(tmp+1,1,1,fp);
        }
    }
    fputc('\n',fp);
    fclose(bootargs);
    system("rm -rf /tmp/bootargs.bin");
    fclose(fp);
    return 1;
}

void changemac(char *a)
{
    char addr[1000]={0};
    FILE *fp;
    fp=fopen("/tmp/bootargs_input.txt","r+");
    for(char tmp=fgetc(fp),count=0,last=0;!feof(fp);tmp=fgetc(fp),count=0)
    {
        while(tmp!='\n')
        {
            addr[count++]=tmp;
            tmp=fgetc(fp);
        }
        addr[count]='\n';
        if(isethaddr(addr))
       {    
            fseek(fp,-count-1,SEEK_CUR);
            strcpy(addr+8,a);
            addr[strlen(addr)]='\n';
            fwrite(addr,count,1,fp);
            fclose(fp);
            return ;
       }
    }
}

int isethaddr(char *str)
{
    char pre[]="ethaddr=";
    for(int i=0;pre[i]!='=';i++)
        if(pre[i]!=str[i])return 0;
    return 1;
}

int isaddr(char *addr)
{
    char uc[6][2];

    if(sscanf (addr, "%2s:%2s:%2s:%2s:%2s:%2s", uc[0],uc[1],uc[2],uc[3],uc[4],uc[5])!= 6)
        return 0;
    else
    {
        for(int i=0;i<6;i++)
        { 
            if(!isdigit(uc[i][0])&&(uc[i][0]<'A'||uc[i][0]>'F'))
                return 0;
            if(!isdigit(uc[i][1])&&(uc[i][1]<'A'||uc[i][1]>'F'))
                return 0;
        }
    }
   return 1;
}

void makebootargs()
{
    system("mkbootargs -s 64 -r /tmp/bootargs_input.txt -o /tmp/bootargs.bin");
}

void flashbootargs()
{
    system("dd if=/tmp/bootargs.bin of=/dev/mmcblk0p2 bs=1024 count=1024");
}
void removefile(){
    system("rm -rf /tmp/bootargs_input.txt /tmp/bootargs.bin");
}
void end(){
    printf("Reboot now?[Y/n]:");
    switch (getchar())
    {
    case 'y':
    case 'Y':
        sleep(1);
        system("reboot");
        break;
    case 'n':
    case 'N':
        return;
    }
}

int main(int argc,char *argv[])
{
    if(argc!=2)
    {
        printf( "\r\nUsage: changemac [MacAddr] :\n"
		        "\r\nExample:changemac AA:BB:CC:DD:EE\n\n");
        exit(0);
    }
    if(!isaddr(argv[1])){
        printf("\r\nMac input error!\n\n");
        exit(0);
    }

    if(readbootargs()){
        changemac(argv[1]);
        makebootargs();
        flashbootargs();
    }
    else 
       printf("\r\nPlease run in sudo\n\n");
    removefile();
    end();
}
