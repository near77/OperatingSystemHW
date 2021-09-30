#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <omp.h>
#include <cstring>
using namespace std;

struct stuple
{
    int length;
    vector <string> s_tuple;
    stuple()
    {
        length = -1;
    }
};

struct variable
{
    string name;
    string value;
};

struct thread_cmd
{
    string thread_no;
    string cmd;
    vector <string> cmd_tuple;
};

vector <stuple> tuple_space(2000);
vector <variable> var_table;
vector <thread_cmd> wait_cmd;

string tuple2string(struct stuple tuple)
{
    string tuple_string = "(";
    for(int j = 0; j < tuple.length; j++)
    {
        if(j == 0){tuple_string += tuple.s_tuple[j];}
        else{tuple_string += ", " + tuple.s_tuple[j];}
    }
    tuple_string += ")";
    return tuple_string;
}

void show_tuple_space(vector <stuple> tuple_space)
{
    printf("------------TUPLE SPACE------------\n");
    for(int i = 0; i < 2000; i++)
    {
        if(tuple_space[i].length != -1)
        {
            string show_tuple = tuple2string(tuple_space[i]);
            printf("%s\n", show_tuple.c_str());
        }
    }
    printf("-----------------------------------\n");
}

void show_var_table(vector <variable> var_table)
{
    printf("-------------VAR TABLE-------------\n");
    printf("------VAR---------------VALUE------\n");
    for(int i = 0; i < var_table.size();i++)
    {
        printf("%8s            %8s\n", var_table[i].name.c_str(), var_table[i].value.c_str());
    }
    printf("-----------------------------------\n");
}

void show_thcmd_buffer(vector <thread_cmd> wait_cmd)
{
    printf("---------THREAD CMD BUFFER---------\n");
    for(int i = 0; i < wait_cmd.size(); i++)
    {
        printf("[%s %s ", wait_cmd[i].thread_no.c_str(), wait_cmd[i].cmd.c_str());
        for(int j = 0; j < wait_cmd[i].cmd_tuple.size();j++)
        {
            printf("%s ", wait_cmd[i].cmd_tuple[j].c_str());
        }
        printf("]\n");
    }
    printf("-----------------------------------\n");
}

vector <int> parse_q_variable(vector <string> thread_tuple)//Return position of ?var
{
    vector <int> q_var_pos;
    for(int i = 0; i < thread_tuple.size(); i++)
    {
        if(thread_tuple[i][0] == '?')
        {
            q_var_pos.push_back(i);
        }
    }
    return q_var_pos;
}

bool search_q_var(vector <string> &thread_tuple)
{
    vector <int> q_var_pos;
    q_var_pos = parse_q_variable(thread_tuple);
    if(q_var_pos.size() == 0){return true;}
    int correct_no = 0;
    int target_pos = -1;
    for(int i = 0; i < tuple_space.size(); i++)
    {
        if(tuple_space[i].length == thread_tuple.size())
        {
            for(int j = 0; j < thread_tuple.size(); j++)
            {
                if(thread_tuple[j][0] == '?')
                {
                    correct_no += 1;
                    continue;
                }
                else
                {
                    if(thread_tuple[j] == tuple_space[i].s_tuple[j]){correct_no += 1;}
                }
            }
            if(correct_no == thread_tuple.size())
            {
                target_pos = i;
                break;
            }
        }
    }
    if(target_pos != -1)
    {
        for(int i = 0; i < q_var_pos.size(); i++)
        {
            struct variable var1;
            var1.name = thread_tuple[q_var_pos[i]].substr(1,strlen(thread_tuple[q_var_pos[i]].c_str()));
            var1.value = tuple_space[target_pos].s_tuple[q_var_pos[i]];
            int var_exist = 0;
            for(int j = 0; j < var_table.size(); j++)
            {
                if(var1.name == var_table[j].name)
                {
                    var_exist = 1;
                    var_table[j].value = var1.value;
                    break;
                }
            }
            if(var_exist == 0){var_table.push_back(var1);}
        }
        return true;
    }
    return false;
}

vector <int> parse_variable(vector <string> thread_tuple)//Return position of var
{
    vector <int> var_pos;
    for(int i = 0; i < thread_tuple.size(); i++)
    {
        if(thread_tuple[i][0] != '\"' && thread_tuple[i][0] != '?')
        {
            double if_var;
            try
            {
                if_var = stod(thread_tuple[i]);
            }
            catch(exception& e)
            {
                if_var = -1;
                var_pos.push_back(i);
            }
        }
        else{continue;}
    }
    return var_pos;
}

vector <string> replace_var(vector <string> thread_tuple)//Replace var with value
{
    vector <int> var_pos = parse_variable(thread_tuple);
    for(int i = 0; i < var_pos.size(); i++)
    {
        int j;
        for(j = 0; j < var_table.size(); j++)
        {
            if(thread_tuple[var_pos[i]] == var_table[j].name)
            {
                thread_tuple[var_pos[i]] = var_table[j].value;
                break;
            }
        }
        if(thread_tuple[var_pos[i]] != var_table[j].value){return {};}
    }
    return thread_tuple;
}


int exec_cmd(string thread_no, string cmd, vector <string> thread_tuple, vector <string> &thread_rw_buffer)
{
    thread_tuple = replace_var(thread_tuple);
    if(thread_tuple.size() == 0){return -1;}
    if(!search_q_var(thread_tuple)){return -1;}
    //--Convert thread no from str to int-------------
    istringstream sno2no(thread_no);
    int th_no = 0;
    sno2no >> th_no;
    //------------------------------------------------
    if(cmd == "out")
    {
        //--Find an empty space to store tuple-------------------
        for(int i = 0; i < 2000; i++)
        {
            if(tuple_space[i].length == -1)
            {
                tuple_space[i].length = thread_tuple.size();
                tuple_space[i].s_tuple = thread_tuple;
                break;
            }
        }
        //-------------------------------------------------------
    }
    else if(cmd == "read" || cmd == "in")
    {
        int if_match = 0;
        //--Check if there exist target tuple--------------------
        for(int i = 0 ; i < 2000; i++)
        {
            if(thread_tuple.size() == tuple_space[i].length)
            {
                if_match = 1;
                for(int j = 0; j < thread_tuple.size(); j++)
                {
                    if(thread_tuple[j][0] == '?'){continue;}
                    if(thread_tuple[j] != tuple_space[i].s_tuple[j])
                    {
                        if_match = 0;
                        break;
                    }
                }
                if(if_match)
                {
                    string th_tuple = tuple2string(tuple_space[i]);//Convert tuple to string
                    thread_rw_buffer[th_no] = th_tuple;//Put tuple string in thread buffer to write file
                    if(cmd == "in")
                    {
                        tuple_space[i].length = -1;//Erase tuple space
                    }
                    break;
                }
            }
        }
        if(if_match == 0){return -1;}
        //------------------------------------------------------
    }
    else
    {
        printf("Unknown command.\n");
    }
    return 1;
}

int main()
{
    string line;
    printf("Client thread number > ");
    int thread_num;
    cin >> thread_num;
    getline(cin, line);
    printf("Thread num: %d\n", thread_num+1);
    omp_set_num_threads(thread_num+1);
    vector <string> thread_rw_buffer(thread_num+1);
    int status;
    #pragma omp parallel
    {
        while(true)
        {
            int tid = omp_get_thread_num();
            if(tid == 0)
            {
                string thread_no;
                string cmd;
                vector <string> thread_tuple;

                for(int i = 0; i < wait_cmd.size(); i++)
                {
                    thread_no = wait_cmd[i].thread_no;
                    cmd = wait_cmd[i].cmd;
                    thread_tuple = wait_cmd[i].cmd_tuple;
                    status = exec_cmd(thread_no, cmd, thread_tuple, thread_rw_buffer);
                    if(status == 1)
                    {
                        wait_cmd.erase(wait_cmd.begin()+i);
                        i--;
                    }
                }
                thread_tuple = {};
                printf("> ");
                getline(cin, line);
                stringstream check1(line); 
                string intermediate; 
                //--Parse line------------------------------------
                check1 >> thread_no;
                if(thread_no == "exit"){exit(0);}
                check1 >> cmd;
                while(check1 >> intermediate) 
                { 
                    thread_tuple.push_back(intermediate); 
                }
                int is_waiting = 0;
                for(int i = 0; i < wait_cmd.size(); i++)
                {
                    if(thread_no == wait_cmd[i].thread_no)
                    {
                        is_waiting = 1;
                        break;
                    }
                }
                if(is_waiting){continue;}
                status = exec_cmd(thread_no, cmd, thread_tuple, thread_rw_buffer);
                if(status != 1)
                {
                    printf("Execute Fail.\n");
                    struct thread_cmd w_cmd;
                    w_cmd.thread_no = thread_no;
                    w_cmd.cmd = cmd;
                    w_cmd.cmd_tuple = thread_tuple;
                    wait_cmd.push_back(w_cmd); 
                }
                show_tuple_space(tuple_space);
                printf("\n");
                show_var_table(var_table);
                printf("\n");
                show_thcmd_buffer(wait_cmd);
            }
            else
            {
                if(thread_rw_buffer[tid] != "")
                {
                    printf("Buffer Content: %s\n", thread_rw_buffer[tid].c_str());
                    thread_rw_buffer[tid] = "";
                }
            }
        }
    }
    return 0;
}