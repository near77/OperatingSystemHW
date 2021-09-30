#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <omp.h>
#include <cstring>
#include <fstream>
#include <unistd.h>

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

vector <stuple> tuple_space;
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
    for(int i = 0; i < tuple_space.size(); i++)
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


int exec_cmd(string thread_no, string cmd, vector <string> thread_tuple, vector <string> &thread_rw_buffer, omp_lock_t *lock)
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
        struct stuple out_tuple;
        out_tuple.length = thread_tuple.size();
        out_tuple.s_tuple = thread_tuple;
        tuple_space.push_back(out_tuple);
        //-------------------------------------------------------
    }
    else if(cmd == "read" || cmd == "in")
    {
        int if_match = 0;
        //--Check if there exist target tuple--------------------
        for(int i = 0 ; i < tuple_space.size(); i++)
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
                    omp_set_lock(lock);
                    while(thread_rw_buffer[th_no] != ""){
                        omp_unset_lock(lock);
                        usleep(10000);
                        omp_set_lock(lock);
                    };
                    thread_rw_buffer[th_no] = th_tuple;//Put tuple string in thread buffer to write file
                    omp_unset_lock(lock);
                    if(cmd == "in")
                    {
                        tuple_space.erase(tuple_space.begin()+i);//Erase tuple space
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
    vector <string> file_name_table;
    for(int i = 0; i <= thread_num; i++)
    {
        string file_name;
        if(i == 0)
        {
            file_name = "Server.txt";
        }
        else
        {
            stringstream tmp;
            file_name = to_string(i) + ".txt"; 
        }
        ofstream file;
        file.open(file_name.c_str());
        file_name_table.push_back(file_name);
    }

    vector <string> thread_rw_buffer(thread_num+1);
    int status;
    omp_lock_t array_lock;
    omp_init_lock(&array_lock);

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

                int tuple_space_size = tuple_space.size();

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
                status = exec_cmd(thread_no, cmd, thread_tuple, thread_rw_buffer, &array_lock);
                if(status != 1)
                {
                    printf("Execute Fail.\n");
                    struct thread_cmd w_cmd;
                    w_cmd.thread_no = thread_no;
                    w_cmd.cmd = cmd;
                    w_cmd.cmd_tuple = thread_tuple;
                    wait_cmd.push_back(w_cmd); 
                }
                int exe_wait_flag = 0;
                for(int i = 0; i < wait_cmd.size(); i++)
                {
                    thread_no = wait_cmd[i].thread_no;
                    cmd = wait_cmd[i].cmd;
                    thread_tuple = wait_cmd[i].cmd_tuple;
                    status = exec_cmd(thread_no, cmd, thread_tuple, thread_rw_buffer, &array_lock);
                    if(status == 1)
                    {
                        exe_wait_flag = 1;
                        wait_cmd.erase(wait_cmd.begin()+i);
                        i--;
                    }
                }

                if(tuple_space.size() == 0)
                {   
                    if(tuple_space.size() != tuple_space_size)
                    {
                        ofstream file;
                        file.open(file_name_table[tid].c_str(), ios::out | ios::app);
                        file << "()\n";
                    }
                    else
                    {
                        if(exe_wait_flag)
                        {
                            ofstream file;
                            file.open(file_name_table[tid].c_str(), ios::out | ios::app);
                            file << "()\n";
                        }
                    }
                    
                    printf("().\n");
                }
                else if(tuple_space.size() != tuple_space_size)
                {   
                    ofstream file;
                    file.open(file_name_table[tid].c_str(), ios::out | ios::app);
                    string tuple_space_str = "(";
                    for(int i = 0; i < tuple_space.size(); i++)
                    {
                        string tmp = tuple2string(tuple_space[i]);
                        if(i != 0)
                        {
                            tuple_space_str+=",";
                        }
                        tuple_space_str+=tmp;
                    }
                    tuple_space_str += ")";
                    file << tuple_space_str.c_str();
                    file << "\n";
                }
                else
                {
                    if(exe_wait_flag)
                    {
                        ofstream file;
                        file.open(file_name_table[tid].c_str(), ios::out | ios::app);
                        string tuple_space_str = "(";
                        for(int i = 0; i < tuple_space.size(); i++)
                        {
                            string tmp = tuple2string(tuple_space[i]);
                            if(i != 0)
                            {
                                tuple_space_str+=",";
                            }
                            tuple_space_str+=tmp;
                        }
                        tuple_space_str += ")";
                        file << tuple_space_str.c_str();
                        file << "\n";
                    }
                }
                
                show_tuple_space(tuple_space);
                printf("\n");
                show_var_table(var_table);
                printf("\n");
                show_thcmd_buffer(wait_cmd);
            }
            else
            {
        
                omp_set_lock(&array_lock);
                if(thread_rw_buffer[tid] != "")
                {
                    ofstream file;
                    file.open(file_name_table[tid].c_str(), ios::out | ios::app);
                    file << thread_rw_buffer[tid].c_str();
                    file << "\n";
                    thread_rw_buffer[tid] = "";
                }
                omp_unset_lock(&array_lock);
            }
        }
    }
    return 0;
}