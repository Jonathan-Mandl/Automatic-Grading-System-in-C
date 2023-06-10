//Yonatan Mandl 211399175
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define BUFSIZE 150


int is_directory(const char* path) {
    struct stat stat_buf;
    if (stat(path, &stat_buf) != 0) {
        // Failed to get file/directory information
        perror("Error in: stat");
        return 0;
    }

    return S_ISDIR(stat_buf.st_mode);
}


int is_regular_file(const char* path) {
    struct stat stat_buf;
    if (stat(path, &stat_buf) != 0) {
        // Failed to get file/directory information
        perror("Error in: stat");
        return 0;
    }

    return S_ISREG(stat_buf.st_mode);
}


void check_valid_paths(char* user_dir,char* input_file,char* output_file)
{
    DIR* dir = opendir(user_dir);
    if (dir == NULL || !is_directory(user_dir)) {
        char *error_dir = "Not a valid directory\n";
        if(write(1, error_dir,strlen(error_dir))==-1){ };
        exit(-1);
    }

    if (closedir(dir)==-1)
    {
        perror("Error in: closedir");   
        exit(-1);
    }

    int fd1 = open(input_file, O_RDONLY);
    if (fd1 == -1 || !is_regular_file(input_file)) {
        // Failed to open file (file cannot be created or permission denied)
        char *error_input="Input file not exist\n";
        if(write(1, error_input,strlen(error_input))==-1){ };    
        exit(-1);
    }

    if (close(fd1)==-1)
    {
        perror("Error in: close");
        exit(-1);

    }

    int fd2 = open(output_file, O_RDONLY);
    if (fd2 == -1 || !is_regular_file(output_file)) {
        // Failed to open file (file cannot be created or permission denied)
        char *error_out="Output file not exist\n";
        if(write(1, error_out,strlen(error_out))==-1){ };    
        exit(-1);
    }
    
    if (close(fd2)==-1)
    {
        perror("Error in: close");
        exit(-1);
    }


}


void parse_configuration_file(int fd,char* user_dir,char* input_file,char* output_file)
{
    // Read first line into user_dir
    int nbytes = 0;
    char c;
    int i = 0;
    while ((nbytes = read(fd, &c, 1)) > 0 && c != '\n'  && c != '\r' && i < BUFSIZE - 1) {
        user_dir[i++] = c;
    }
    if(nbytes==-1)
    {
        perror("Error in: read");
        exit(-1);
    }
    user_dir[i] = '\0';  // Replace newline with null terminator

    // Read second line into input_file
    i = 0;
    while ((nbytes = read(fd, &c, 1)) > 0 && c != '\n' && c != '\r' && i < BUFSIZE - 1) {
        input_file[i++] = c;
    }
    if(nbytes==-1)
    {
        perror("Error in: read");
        exit(-1);
    }
    input_file[i] = '\0';  // Replace newline with null terminator


    // Read third line into output_file
    i = 0;
    while ((nbytes = read(fd, &c, 1)) > 0 && c != '\n' && c != '\r' && i < BUFSIZE - 1) {
        output_file[i++] = c;
    }
    if(nbytes==-1)
    {
        perror("Error in: read");
        exit(-1);
    }
    output_file[i] = '\0';  // Replace newline with null terminator


}

void add_to_csv(char *student_name, char* score, char *comment) {
    char filename[] = "results.csv";
    int fd = open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("Error in: open");
    }

    char line[BUFSIZE];
    strcpy(line,student_name);
    strcat(line,",");
    strcat(line,score);
    strcat(line,",");
    strcat(line,comment);
    strcat(line,"\n");


    if (write(fd, line, strlen(line)) == -1) {
        
    }

    if (close(fd) == -1) {
        perror("Error in: close");
    }
}



int compile_c_program(char* c_file_path,char* out_file_path, char* student_name)
{

    pid_t pid = fork();
    if (pid < 0) {
        // Fork failed
        perror("Error in: fork");
        return 0;

    } else if (pid == 0) {
        // Child process
        int fd_err = open("errors.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (fd_err == -1) {
            perror("Error in: open");
            exit(-1);
        }
        if (dup2(fd_err, 2) == -1) {
            perror("Error in: dup2");
            exit(-1);
        }

        char* args[] = {"gcc", "-o", out_file_path, c_file_path, NULL};
        execvp("gcc", args);
        perror("Error in: execvp");
        exit(-1);
    } else {
    // Parent process
    int status;
    if (wait(&status)==-1)
    {
        perror("Error in: wait");
        return 0;
    }
    int access_return=access(out_file_path, F_OK);
    if (access_return == 0) {
    // out file exists. means program was compiled successfully.
    return 1;
    } 
    else if (access_return == -1) {
    //access failed.
    if (errno == ENOENT) {
        //This errno flag means file does not exist. this means program was not compiled
        add_to_csv(student_name,"10","COMPILATION_ERROR");
        return 0;
    } else {
        // there was an error in access.
        perror("Error in: access");
        return 0;
    }       
    }
    else{ //other access error...
        return 0;
    }
    }

}


int run_c_program(char* input_file, char* out_file_path, char* subdir_path, char*student_name )
{
    char program_output_file[BUFSIZE];
    strcpy(program_output_file, subdir_path);
    strcat(program_output_file, "/");
    strcat(program_output_file,"program_output.txt");

    int run=0;

    pid_t pid = fork();
    if (pid < 0) {
    // Fork failed
    perror("Error in: fork");
    goto cleanup;

    } else if (pid == 0) {
    // Child process

    // Redirect stdout and stderr to files
    int fd_err = open("errors.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd_err == -1) {
        perror("Error in: open");
        exit(-1);
    }
    if (dup2(fd_err, 2) == -1) {
        perror("Error in: dup2");
        exit(-1);
    }

    //redirect stdin to input file.

    int fd_in = open(input_file, O_RDONLY);
    if (fd_in == -1) {
        perror("Error in: open");
        exit(-1);
    }
    if (dup2(fd_in, 0) == -1) {
        perror("Error in: dup2");
        exit(-1);
    }

    //redirect stdout to output file.
    int fd_out = open(program_output_file, O_WRONLY | O_CREAT, 0644);
    if (fd_out == -1) {
        perror("Error in: open");
        exit(-1);
    }
    if (dup2(fd_out, 1) == -1) {
        perror("Error in: dup2");
        exit(-1);
    }

    alarm(5);

    char* args[] = {"./", out_file_path, NULL};
    execvp(out_file_path, args);
    perror("Error in: execvp");
    exit(-1);

} else {
    // Parent process
    int status;
    if (wait(&status) == -1) {
        perror("Error in: wait");
        goto cleanup;
    }

    
    if (WIFEXITED(status)) {
        //child exited normally
        int exit_status = WEXITSTATUS(status);
        if(exit_status != -1) {
            // child successfully completed run and did not exit with -1.
            run=1;
        }
    }
    else if(WIFSIGNALED(status)) {
    // Program was terminated by a signal
    if (WTERMSIG(status) == SIGALRM) {
    // Program took too long to run
    add_to_csv(student_name,"20","TIMEOUT");
    } 
    }
    else{
        //child did not exit normally.
    }
    
    cleanup:
        if (remove(out_file_path)==-1)
        {
            perror("Error in: remove");
        }

    return run;

}
}

void compare_output(char* student_name,char* subdir_path, char* output_file)
{
    char program_output_file[BUFSIZE];
    strcpy(program_output_file, subdir_path);
    strcat(program_output_file, "/");
    strcat(program_output_file,"program_output.txt");

    pid_t pid = fork();
    if (pid < 0) {
        // Fork failed
        perror("Error in: fork");
        goto cleanup;

    } else if (pid == 0) {
        int fd_err = open("errors.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (fd_err == -1) {
            perror("Error in: open");
            exit(-1);
        }
        if (dup2(fd_err, 2) == -1) {
            perror("Error in: dup2");
            exit(-1);
        }
        char* args[] = {"./comp.out", program_output_file, output_file, NULL};
        execvp("./comp.out", args);
        perror("Error in: execvp");
        exit(-1);
    }
    else{
    int status1;
    if (waitpid(pid, &status1, 0) == -1) {
        perror("Error in: waitpid");
        goto cleanup;
    }

    if (WIFEXITED(status1)) {
        // Child process exited normally
        int return_value = WEXITSTATUS(status1);
        if(return_value==2){
            add_to_csv(student_name,"50","WRONG");
        }
        else if (return_value==3)
        {
            add_to_csv(student_name,"75","SIMILAR");
        }
        else if(return_value==1){
            add_to_csv(student_name,"100","EXCELLENT");
        }
        else{
            //abnormal exit status.
        }  
    }

    cleanup:
        if (remove(program_output_file)==-1)
        {
            perror("Error in: remove");
        }
}
}

void scan_directory(const char* user_dir, char* input_file, char* output_file) {
    DIR* dir = opendir(user_dir);
    if (dir == NULL) {
        perror("Error in: opendir");
        exit(-1);
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            // Skip . and .. entries
            continue;
        }
        char* student_name = entry->d_name;

        char subdir_path[BUFSIZE];
        strcpy(subdir_path, user_dir);
        strcat(subdir_path, "/");
        strcat(subdir_path, student_name);

        if(!is_directory(subdir_path))
        {
            continue;
        }

        DIR* subdir = opendir(subdir_path);
        if (subdir == NULL) {
            // Failed to open subdirectory
            perror("Error in: opendir");
            continue;
        }

        int has_c_file = 0;
        struct dirent* c_entry;
        while ((c_entry = readdir(subdir)) != NULL) {
            if (strcmp(c_entry->d_name, ".") == 0 || strcmp(c_entry->d_name, "..") == 0) {
                // Skip . and .. entries
                continue;
            }
            int len = strlen(c_entry->d_name);

            if (len >= 2 && strcmp(c_entry->d_name + len - 2, ".c") == 0) {
                char c_file_path[BUFSIZE];
                strcpy(c_file_path, subdir_path);
                strcat(c_file_path, "/");
                strcat(c_file_path, c_entry->d_name);

                if (is_directory(c_file_path)) {
                    // If the file is not a regular file, continue.
                    continue;
                }

                // Found a C file
                has_c_file = 1;

                char out_file_path[BUFSIZE];
                strcpy(out_file_path, c_file_path);
                out_file_path[strlen(out_file_path) - 2] = '\0';
                strcat(out_file_path, ".out");

                int compiled = compile_c_program(c_file_path, out_file_path, student_name);

                int run = 0;

                if (compiled) {
                    run = run_c_program(input_file, out_file_path, subdir_path, student_name);
                }

                if (run) {
                    compare_output(student_name, subdir_path, output_file);
                }
            }
        }

        if (!has_c_file) {
            add_to_csv(student_name, "0", "NO_C_FILE");
        }

        if (closedir(subdir) == -1) {
            perror("Error in: closedir");
        }
    }

    if (closedir(dir) == -1) {
        perror("Error in: closedir");
        exit(-1);
    }
}




int main(int argc, char *argv[]) {

    if (argc != 2) {
        perror("Invalid number of arguments");
        exit(-1);
    }

    int fd = open(argv[1], O_RDONLY);  // Open config file for reading
    if (fd == -1) {
        perror("Error in: open");
        exit(-1);
    }

    char user_dir[BUFSIZE];
    char input_file[BUFSIZE];
    char output_file[BUFSIZE];

    parse_configuration_file(fd,user_dir,input_file,output_file);

    // Close configuration file
    if(close(fd))
    {
        perror("Error in: close");
        exit(-1);
    } 

    check_valid_paths(user_dir,input_file,output_file);

    scan_directory(user_dir,input_file,output_file);

    return 0;
}

