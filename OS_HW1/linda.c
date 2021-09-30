#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#define READBUF_SIZE 1024
#define TOKENBUF_SIZE 256
#define DELI " "

char **parse_line(char *line, char * delimeters)
{
    int bufsize = TOKENBUF_SIZE, position = 0;
    char **tokens = (char **)malloc(sizeof(char*) * bufsize);
    char *token;
    if(!tokens)
    {
        printf("Allocation Fail.\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, delimeters);
    while(token != NULL)
    {
        tokens[position] = token;
        position++;
        if(position >= bufsize){
            bufsize += TOKENBUF_SIZE;
            tokens = (char **)realloc(tokens, sizeof(char*) * bufsize);
            if(!tokens)
            {
                printf("Allocation Fail.\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, delimeters);
    }
    tokens[position] = NULL;
    return tokens;
}

char *read_line()
{
    int bufsize = READBUF_SIZE;
    int line_position = 0;
    int c;
    char *buffer = malloc(sizeof(char)*bufsize);
    if(!buffer)
    {
        printf("Allocation Fail.\n");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        c = getchar();
        if(c == '\n')
        {
            buffer[line_position] = '\0';
            return buffer;
        }
        else if(c == EOF)
        {
            printf("\n");
            exit(0);
        }
        else
        {
            buffer[line_position] = c;
        }
        line_position++;
        if(line_position >= bufsize)
        {
            bufsize += READBUF_SIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer)
            {
                printf("Allocation Fail.\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

struct tuple
{
    int length;
    char** s_tuple;
};

int main()
{
    char* line;
    char** args;
    int cmd_no = 0;

    int thread_num;
    printf("Input Number of Thread:");
    line = read_line();
    thread_num = atoi(line)+1;
    printf("Thread num: %d\n", thread_num);
    omp_set_num_threads(thread_num);

    struct tuple tuple_space[2000];
    memset(&tuple_space, 0, 2000*sizeof(struct tuple));
    for(int i = 0; i < 2000; i++)
    {
        tuple_space[i].length = -1;
        tuple_space[i].s_tuple = NULL;
    }

    do
    {
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();

            if(tid == 0)//Server thread
            {
                //--Parse line--------------------------------
                printf("> ");
                line = read_line();
                args = parse_line(line, DELI);
                //--------------------------------------------

                //--Check cmd length--------------------------
                int cmd_length = 0;
                while(args[cmd_length]){cmd_length++;}
                //--------------------------------------------

                //--Record tid&cmd----------------------------
                int thread_id = atoi(args[0]);
                char* thread_cmd;
                if(args[1]){thread_cmd = args[1];}
                else{exit(0);}
                #ifdef dbg
                printf("Tuple thread id: %d\n", thread_id);
                printf("Tuple cmd: %s\n", thread_cmd);
                #endif
                //--------------------------------------------
                
                //--Record cmd tuple--------------------------
                int tuple_length = cmd_length-2;
                char* cmd_tuple[tuple_length+1];
                for(int i = 2; i < cmd_length; i++)
                {
                    cmd_tuple[i-2] = args[i];
                    #ifdef dbg
                    printf("Tuple parameter: %s\n", cmd_tuple[i-2]);
                    #endif
                }
                cmd_tuple[tuple_length] = 0;
                //--------------------------------------------

                if(strcmp(thread_cmd, "out") == 0)
                {
                    for(int i = 0; i < 2000; i++)
                    {
                        if(tuple_space[i].length == -1)
                        {
                            tuple_space[i].length = tuple_length;
                            for(int j = 0; j <= tuple_length; j++)
                            {
                                printf("j: %d\n", j);
                                tuple_space[i].s_tuple[j] = cmd_tuple[j];
                            }
                            
                            break;
                        }
                    }
                }
                for(int i = 0; i < 2000; i++)
                {
                    if(tuple_space[i].length != -1)
                    {
                        int j = 0;
                        printf("(");
                        while(tuple_space[i].s_tuple[j])
                        {
                            if(j == 0){printf("%s", tuple_space[i].s_tuple[j]);}
                            else{printf(", %s", tuple_space[i].s_tuple[j]);}
                            j++;
                        }
                        printf(")\n");
                    }
                }
            }
        }
    } while (strcmp(line, "exit") != 0);
    
    return 0;
}