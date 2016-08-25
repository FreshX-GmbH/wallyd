// for embedding ressources we use a memfs which is a combination of tar and fmemopen
// Not yet windows compatible

#include <stdio.h> 
#include <fcntl.h> 
#include <string.h> 
#include <sys/mman.h> 
#include "util.h"
#include "plugins.h"
#include "miniz.h"

int openVFS(pluginHandler *ph, char *file){
    int fd=open( file, O_RDONLY );
    if(fd == -1) {
        slog(LVL_QUIET,ERROR,"Could not open config data at : %s",file);
        return false;
    }
    ph->VFSSize = (long)lseek(fd,0,SEEK_END);
    ph->VFSName = file;
    ph->VFSOpen = true;
    close(fd);
    return true;
}

void *getVFSFile(pluginHandler *ph,char *file, size_t *pSize){
    char *ptr = NULL;//mz_zip_extract_archive_file_to_heap(file, ph->VFSName, pSize, 0);
    if(ptr == 0){
        slog(DEBUG,DEBUG,"Could not find %s in VFS",file);
        return NULL;
    }
    return ptr;
}

//struct tar {
//  char name[100];               /*   0 */
//  char mode[8];                 /* 100 */
//  char uid[8];                  /* 108 */
//  char gid[8];                  /* 116 */
//  char size[12];                /* 124 */
//  char _padding[376];
////  char mtime[12];               /* 136 */
////  char chksum[8];               /* 148 */
////  char typeflag;                /* 156 */
////  char linkname[100];           /* 157 */
////  char magic[6];                /* 257 */
////  char version[2];              /* 263 */
////  char uname[32];               /* 265 */
////  char gname[32];               /* 297 */
////  char devmajor[8];             /* 329 */
////  char devminor[8];             /* 337 */
////  char prefix[155];             /* 345 */
//} *tar;
//
//int list_mem_tar( struct tar *tar, char **start, long length ){
//    for( ; tar->name[0]; tar+=1+(length+511)/512 ){
//        sscanf( tar->size, "%o", length);
//        slog(DEBUG,DEBUG, "%d / %s",tar->size,tar->name);
//        //*start = (char*)(tar+1);
//    }
//    return 0;
//}
//
//int is_file_in_tar( struct tar *tar, char *name, char **start, int *length ){
//    for( ; tar->name[0]; tar+=1+(*length+511)/512 ){
//        sscanf( tar->size, "%o", length);
//        slog(DEBUG,DEBUG, tar->name);
//        if( !strcmp(tar->name,name) ){ *start = (char*)(tar+1); return 1; }
//    }
//    return 0;
//}
//
//void *mapDataFile(char *file, long *size){
//    void *start = NULL;
//    
//    int fd=open( file, O_RDONLY );
//    if(fd == -1) {
//      slog(LVL_QUIET,ERROR,"Could not open config data at : %s",file);
//      return 0;
//    }
//    
//    *size = (long)lseek(fd,0,SEEK_END);
//    if(*size == -1){
//      slog(LVL_QUIET,ERROR,"Could not seek config data at : %s",file);
//      return 0;
//    }
//    lseek(fd,0,0);
//
//    start = mmap(NULL, *size, PROT_READ, MAP_PRIVATE, fd, 0);
//    slog(DEBUG,DEBUG,"VFS %s open, %d bytes, data at 0x%x",file,*size,start);
//
//    if(start == MAP_FAILED){
//        slog(LVL_QUIET,ERROR,"Could not mmap %s into memory, err %d",errno);
//    }
//    return start;
//}
//
//int getMemFile(char *tar, char *name){
//    //  tar=mmap(NULL, 808960, PROT_READ, MAP_PRIVATE, fd, 0);
//    char *start; int length;
//    slog(DEBUG,DEBUG,"VFS : %s : %.*s",name,length,start);
//    return is_file_in_tar(tar,name,&start,&length);
//}
