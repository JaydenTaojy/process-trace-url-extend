#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

const int long_size = sizeof(long);

void getdata(pid_t child, long addr,
             char *str, int len)
{   char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * 4,
                          NULL);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * 4,
                          NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
}
void putdata(pid_t child, long addr, char *str, int len)
{   char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    //printf(laddr);
    while(i < j) {
        memcpy(data.chars, laddr, long_size);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 4, data.val);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 4, data.val);
    }
}
int main(int argc, char *argv[])
{
   pid_t child;
   child = fork();
    if (argc!=3){
       printf("the argc is %d\n", argc);
       exit(-1);
    }
    char *filename="dowfile.html\0";
    int filelen=strlen(filename);
    int runself = 0;
    int stringlen=strlen(argv[2]); //find the size of http
    char *child_command=(char *)calloc(2048, sizeof(char));
    char *self_command=(char *)calloc(2048, sizeof(char));
    char *empty=(char*)malloc(stringlen);
    memset(empty, '\0', stringlen);
    //printf("stringlen is %d, filelen is %d.\n", stringlen, filelen);
   strcpy (self_command, "wget -O dowfile.html ");
   strcat (self_command, argv[2]);
   strcpy (child_command, argv[1]);
   strcat (child_command, " ");
   strcat (child_command, argv[2]);
    
   if(child == 0) {
      ptrace(PTRACE_TRACEME, 0, NULL, NULL);
      //system(child_command);
       if (execlp(argv[1], argv[1], argv[2], NULL)==-1){
           perror("execl");
           exit(-1);
       }
   }
   else {
      long orig_eax;
      long params;
      //char *filepath;
      int status;
      char *str;
      int insyscall=0; //for delete the file;
      char dest[10];
      while(1) {
         wait(&status);
         if(WIFEXITED(status))
             break;
         orig_eax = ptrace(PTRACE_PEEKUSER,
                           child, 4 * ORIG_EAX,
                           NULL);
        //printf("The child made a system call %ld\n", orig_eax);
          
         if(orig_eax == SYS_stat64) {
            if(insyscall == 0) {
               insyscall = 1;
                if (runself==0){
                    runself=1;
                    system(self_command);}
               params = ptrace(PTRACE_PEEKUSER,
                                  child, 4 * EBX,
                                  NULL);
               str = (char *)calloc((stringlen+1), sizeof(char));
               getdata(child, params, str,
                       stringlen);
               strncpy(dest, str, 4);
               dest[4]=0;
               if (dest[0]=='h'&& dest[1]=='t'&& dest[2]=='t'&& dest[3]=='p'){
               putdata(child, params, empty,stringlen);
               putdata(child, params, filename, filelen);}
            }
            else {
               insyscall=0;
            }
         }
          if(orig_eax == SYS_open) {
             if(insyscall == 0) {
                insyscall=1;
                if (runself==0){
                runself=1;
                system(self_command);}
                params = ptrace(PTRACE_PEEKUSER, child, 4*EBX,
                                   NULL);
                str = (char *)calloc((stringlen+1), sizeof(char));
                getdata(child, params, str,stringlen);
                //printf(str);
                //printf("\n");
                strncpy(dest, str, 4);
                dest[4]=0;
                if (dest[0]=='h'&& dest[1]=='t'&& dest[2]=='t'&& dest[3]=='p'){
                insyscall=1;
                putdata(child, params, empty,stringlen);
                putdata(child, params, filename,filelen);}
             }
             else {
                insyscall=0;
                params = ptrace(PTRACE_PEEKUSER, child, 4*EBX, NULL);
                str = (char *)calloc((stringlen+1), sizeof(char));
                getdata(child, params, str, stringlen);
                //printf(str);
                //printf("\n");
                strncpy(dest, str, 4);
                dest[4]=0;
             }
          }
      ptrace(PTRACE_SYSCALL, child, NULL, NULL);
      }
   }
    if(runself==1){
    if (remove(filename) == 0)
      printf("Deleted successfully.\n");
    else
        printf("Deleted fail.\n");}
   return 0;
}
