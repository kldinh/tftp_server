//Kevin Dinh

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream> 
#include <fstream>
#include <string>
#include <cstring>
#include <sys/param.h>
#include <time.h>
#include <sstream>
#include <dirent.h>
    

using namespace std; 

#define BUFFER_SIZE 516

char output_buffer[512];
char contents_output_buffer[512];
char writebuffer[512];
string list_contents;
char ack[4] = {0,4,0,};


int lseek_flag; 
int lseek_index;

socklen_t ClientLength;
    struct sockaddr_in ServerAddress, ClientAddress;

void error(const char *message)
{
	perror(message);
	exit(1);
}

int SocketFileDescriptor;

void SignalHandler(int param){
    close(SocketFileDescriptor);
    exit(0);
}


int DisplayMessage(char *data, int length){
    int Offset = 0;
    int Index;
    int type;
    int flag = 0;


    //cout << "data: " << data << endl; 
    while(Offset < length)
    {
        	//printf("Offset: ");
        	//printf("%04X ", Offset);
            //cout << endl;
       for(Index = 0; Index < 16; Index++){
            if((Offset + Index) < length)
            {   

                if(data[Offset+Index] == 42)
                    {
                        lseek_flag = 1;
                        lseek_index = (Offset+Index);
                    }

                if(Index == 0)
                {
                    //cout << "OP Code: ";
                    if(data[Offset + Index + 1] == 1)
                        type = 1;
                    if(data[Offset + Index + 1] == 2)
                        type = 2;
                    if(data[Offset + Index + 1] == 3)
                        type = 3;
                    if(data[Offset + Index + 1] == 4)
                        type = 4;
                    if(data[Offset + Index + 1] == 5)
                        type = 5; //error
                }
                if(Index == 2)
                   // cout << endl << "Filename: ";
                if(data[Offset + Index] == 0 && Index > 3 && flag == 0)
                    {
                       // cout << endl << "The Rest: ";
                        flag = 1;
                    }


               // printf("%02X ",data[Offset + Index]);
            }
            else{
               // printf("   ");
            }
        }

       // cout << endl;
    
       for(Index = 0; Index < 16; Index++)
        {
            if((Offset + Index) < length)
            {
               if((' ' <= data[Offset + Index])&&(data[Offset + Index] <= '~'))
                {
                   // printf("%c",data[Offset + Index]);
                }
                else{
                   // printf(".");
                }
            }
            else{
              //  printf(" ");
            }
        }
        //printf("\n");
        Offset += 16;
    }

    return type;
}

//returns where the filename ends in 
int parse(char *data, int length)
{   
    int flag = 0;
    int end_file_name = 0;

   // cout << "Buffer: " << endl;
     for(int i = 0; i < 16; i++)
    {
     //   printf("Buffer at %d = %c \n", i, data[i]);
        if(i > 4)
        {
            if(data[i] == 0 && flag != 1)
                {
                    end_file_name = i;
                    flag = 1;
                }
        }

    }

    //cout << "end_file_name: " << end_file_name << endl;

    return end_file_name;

}

void offset_read(const char* filename, int offset_value)
{
    ifstream myReadFile;
    myReadFile.open(filename);

    char read_data[10000];
    char data;
    int count = 0;
    int Result;
    int num_sends_needed;

    if((myReadFile.is_open()))
    {
        cout << "Opened File Okay..." << endl;
        while( !myReadFile.eof()  )
        {
            myReadFile.get(data);

            if(count >= offset_value)
                read_data[count] = data;

            count++;

        }
    }
    else
        {   cout << "ERROR OPENING FILE using lseek" << endl;
        } 

    myReadFile.close();

    cout << "Count: " << count << endl;

    num_sends_needed = (count / 512) + 1;
    cout << "num_sends_needed: " << num_sends_needed << endl;

    output_buffer[0] = 0; 
    output_buffer[1] = 3; // 3 = DATA TYPE PACKET
    output_buffer[2] = 0; //
    output_buffer[3] = 0;  // BLOCK # = 1

   // for(int i = 0; i<count; i++)
      // output_buffer[i+4] = (int)read_data[i];

    cout << endl;

    for(int i = 0; i< num_sends_needed; i++)
    {
        if(i == (num_sends_needed - 1) )// last packet to send
        {
            cout << "got in last packet to send" << endl;
            char final_output_buffer[count+4]; //make packet that is nto 512 in size

            final_output_buffer[0] = 0;
            final_output_buffer[1] = 3;
            final_output_buffer[2] = 0;
            final_output_buffer[3] = (1+i);

            //count = 94

            for(int m = 0; m < count; m++)
                final_output_buffer[m+4] = read_data[(508*i) + m]; //storing the final values 

            //cout << "Count: " << count << endl;
            /*for(int k = 0; k < count +4; k++)
            {
                printf("output_buffer[%d] = %c\n", k,  final_output_buffer[k]);
            }*/


            Result = sendto(SocketFileDescriptor, final_output_buffer , (count+3), 0, (struct sockaddr *)&ClientAddress, ClientLength);
            if(0 > Result)
                cout << "ERROR sending to client" << endl;

        }
        
        else //not the last packet to send
        {   
             output_buffer[3] = i +1;
            cout << "still packets to send" << endl;
            for(int j = 0; j < 508; j++)
                output_buffer[j+4] = read_data[j + (i * 508)];


                 for(int k = 0; k < 3; k++)
                   // printf("output_buffer[%d] = %d\n", k,  output_buffer[k]);


                for(int k = 4; k < 512; k++)
                    //printf("output_buffer[%d] = %c\n", k,  output_buffer[k]);


            Result = sendto(SocketFileDescriptor, output_buffer, 516, 0, (struct sockaddr *)&ClientAddress, ClientLength);
                 if(0 > Result)
                cout << "ERROR sending to client" << endl;

            count = count - 508;
        }


        //output_buffer[3] = i +1;
        cout << "COUNT: " << count << endl;
    
    }
}



void openandreadfile(const char * filename)
{
    ifstream myReadFile;
    myReadFile.open(filename);

    char read_data[10000];
    char data;
    int count = 0;
    int Result;
    int num_sends_needed;

    if((myReadFile.is_open()))
    {
        cout << "Opened File Okay..." << endl;
        while( !myReadFile.eof()  )
        {
            myReadFile.get(data);
            read_data[count] = data;
            count++;
        }
    }
    else
        {   cout << "ERROR OPENING FILE" << endl;
        } 

    myReadFile.close();

    cout << "Count: " << count << endl;

    num_sends_needed = (count / 512) + 1;
    cout << "num_sends_needed: " << num_sends_needed << endl;

    output_buffer[0] = 0; 
    output_buffer[1] = 3; // 3 = DATA TYPE PACKET
    output_buffer[2] = 0; //
    output_buffer[3] = 0;  // BLOCK # = 1

   // for(int i = 0; i<count; i++)
      // output_buffer[i+4] = (int)read_data[i];



    cout << endl;

    for(int i = 0; i< num_sends_needed; i++)
    {
        if(i == (num_sends_needed - 1) )// last packet to send
        {
            cout << "got in last packet to send" << endl;
            char final_output_buffer[count+4]; //make packet that is nto 512 in size

            final_output_buffer[0] = 0;
            final_output_buffer[1] = 3;
            final_output_buffer[2] = 0;
            final_output_buffer[3] = (1+i);

            //count = 94

            for(int m = 0; m < count; m++)
                final_output_buffer[m+4] = read_data[(508*i) + m]; //storing the final values 

            //cout << "Count: " << count << endl;
            /*for(int k = 0; k < count +4; k++)
            {
                printf("output_buffer[%d] = %c\n", k,  final_output_buffer[k]);
            }*/


            Result = sendto(SocketFileDescriptor, final_output_buffer , (count+3), 0, (struct sockaddr *)&ClientAddress, ClientLength);
            if(0 > Result)
                cout << "ERROR sending to client" << endl;

        }
        
        else //not the last packet to send
        {   
             output_buffer[3] = i +1;
            cout << "still packets to send" << endl;
            for(int j = 0; j < 508; j++)
                output_buffer[j+4] = read_data[j + (i * 508)];


                // for(int k = 0; k < 3; k++)
                   // printf("output_buffer[%d] = %d\n", k,  output_buffer[k]);


                //for(int k = 4; k < 512; k++)
                   // printf("output_buffer[%d] = %c\n", k,  output_buffer[k]);


            Result = sendto(SocketFileDescriptor, output_buffer, 516, 0, (struct sockaddr *)&ClientAddress, ClientLength);
                 if(0 > Result)
                cout << "ERROR sending to client" << endl;

            count = count - 508;
        }


        output_buffer[3] = i +1;
        cout << "COUNT: " << count << endl;
    
    }
}

void creatandwritefile(const char *filename, char *data, int length)
{
    int Result;
    int length2;

    fstream file(filename, fstream::in | fstream::out | fstream::trunc);

    //cout << "first packet length" << length << endl;

   for(int i = 4; i < length; i++)
        file << data[i];

    /*for(int y=0; y<length; y++)
        printf("first_data[%d] = %c\n", y, data[y]);*/

    int block_num = ack[3];
    block_num++;
    ack[3] = block_num;
    int counter = 0;

    
   Result = sendto(SocketFileDescriptor, ack, 4, 0, (struct sockaddr *)&ClientAddress, ClientLength);
            if(0 > Result)
                 cout << "ERROR sending to client" << endl;

   if(length == 516)// if receved a full buffer, there more packets to be sent
    {   
         cout << "reached here" << endl;

         length2 = 516;
    
        while(length2 == 516)
        {
            Result = recvfrom(SocketFileDescriptor, writebuffer, 516, 0, (struct sockaddr *)&ClientAddress, &ClientLength);  
            length2 = Result;

            cout << "Packet Legnth: " << length2 << endl;

            if(counter == 0)
            {
                for(int k=0; k < length2;k++)
                printf("seconddata[%d] = %c \n", k, writebuffer[k]);
            }

            for(int j = 4; j<length2;j++)
            {
                file << writebuffer[j];
            }

             int block_num2 = ack[3];
             block_num2++;
             //cout << "block_num2: " << block_num2 << endl;
             ack[3] = block_num2;

            Result = sendto(SocketFileDescriptor, ack, 4, 0, (struct sockaddr *)&ClientAddress, ClientLength);
            if(0 > Result)
                 cout << "ERROR sending to client" << endl;

             counter++;
             //cout << "counter: " << counter << endl;

             //cout << "length2: " << length2 << endl;
         }

    }


    file.close();

}


void makedirectory()
{
     struct dirent *direct;
     struct stat statbuf;
     DIR *dir = opendir(".");
     if(dir == NULL)
        cout << "error opening directory..." << endl;

    while((direct = readdir(dir)) != NULL)
            {
               unsigned char type = direct->d_type;

                stat(direct->d_name, &statbuf);
                long size = statbuf.st_size;

               list_contents.append(direct->d_name);

               if(type == 4)
                    list_contents.append("/|");
                else
                    list_contents.append("|");

                if(type == 4)
                    list_contents.append("0");
                else 
                {
                    stringstream ss;
                    ss << size;
                    list_contents.append(ss.str());
                }
                   


                list_contents.append("\n");

            }
    //        cout << list_contents << endl;
    
    for(unsigned int i = 0; i < list_contents.length();i++)
    {
        char x = list_contents.at(i);
        int ascii_value = int(x);
        contents_output_buffer[i+4] = ascii_value; 
      //  printf("%c ", contents_output_buffer[i]);
    }

    contents_output_buffer[1] = 3; // 3 = DATA TYPE PACKET
    contents_output_buffer[2] = 0; //
    contents_output_buffer[3] = 1;  // BLOCK # = 1

   // cout << endl << "contents buffer: " << endl;

    /* for(unsigned int i = 0; i < list_contents.length();i++)
    {
        printf("%c ", contents_output_buffer[i]);
    }*/

     list_contents.clear();

}

void sendfirstack()
{
    ack[3] = 0;
    sendto(SocketFileDescriptor, ack, BUFFER_SIZE, 0, (struct sockaddr *)&ClientAddress, ClientLength);
}

int main(int agrc, char *argv[])
{
	int PortNumber;
	char Buffer[BUFFER_SIZE]; //data of whole packet
    char Buffer2[BUFFER_SIZE]; //for filename of just normal put and get
    char Buffer3[BUFFER_SIZE]; //for filename when there is *
    char Buffer4[BUFFER_SIZE]; //for the value after the *
	int Result;
	time_t RawTime;
    struct tm TimeOut;
    int list_dir = 0;

	char rootdir[256];
	//string rootdir;

	if(agrc < 2)
	{
		PortNumber = 49999;
		getcwd(rootdir, 256);

		cout << "Portnumber: " << PortNumber << endl;
		cout << "rootdir: " << rootdir << endl;
	}
	else 
	{
		PortNumber = atoi(argv[1]);
		cout << "Portnumber: " << PortNumber << endl;
    	if((PortNumber < 49152)||(PortNumber > 65535)){
        	fprintf(stderr,"Port %d is an invalid port number\n",PortNumber);
  			exit(-1);
    	}
    }

    // Create UDP/IP socket
    SocketFileDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    	if(SocketFileDescriptor < 0){
        error("ERROR opening socket");
    }

    signal(SIGTERM, SignalHandler);
    signal(SIGINT, SignalHandler);
    signal(SIGUSR1, SignalHandler);
    

    // Setup ServerAddress data structure
    bzero((char *) &ServerAddress, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_addr.s_addr = INADDR_ANY;
    ServerAddress.sin_port = htons(PortNumber);

    // Binding socket to port
    if(0 > bind(SocketFileDescriptor, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress))){ 
        error("ERROR on binding");
    }


    cout << endl;
   while(1)
    {
        ClientLength = sizeof(ClientAddress);
        bzero(Buffer, BUFFER_SIZE);
        // Receive message from client
        Result = recvfrom(SocketFileDescriptor, Buffer, BUFFER_SIZE, 0, (struct sockaddr *)&ClientAddress, &ClientLength);
        if(0 > Result){
            error("ERROR receive from client");
        }
        
        time ( &RawTime );
        localtime_r(&RawTime, &TimeOut);
        
       	printf("Received Message %d bytes @ %04d/%02d/%02d %02d:%02d:%02d\n", Result, TimeOut.tm_year+1900, TimeOut.tm_mon+1, TimeOut.tm_mday, TimeOut.tm_hour, TimeOut.tm_min, TimeOut.tm_sec);
       	cout << endl;


        cout << endl;
        //cout << "Result: " << Result << endl;

       // cout << "What the Server Received: " << endl;
       // for(int i = 0; i < 256; i++)
        //{
         //   printf("%d ", Buffer[i]);
        //}

        cout << endl;
        int type = DisplayMessage(Buffer, Result);

        int marker = parse(Buffer, Result);

        for(int i = 2; i < marker+1; i++)
            {
                Buffer2[i-2] = Buffer[i];
                if(Buffer[2] == 63 && Buffer[3] == 0)
                    list_dir = 1;
            }

        const char *filename = Buffer2; //normal filename
        cout << "FILENAME: " << filename << endl;


        if(lseek_flag == 1) //parsing when there
        {

            for(int j = 2; j < lseek_index; j++)
            {
                Buffer3[j-2] = Buffer[j];
            }

             int n = 0;



            for(int m = lseek_index+1; m < marker; m++)
            {
                Buffer4[n] = Buffer[m];
                n++;
            }
        
            
            n = 0;
            
        }

        


        if(type == 1)//read request
        {   
            cout << "THIS IS A READ REQUEST!" << endl; 

            if(lseek_flag == 1)
            {
                cout << "request file by an offset" << endl;

                const char *filename_no_star = Buffer3;

                cout << "Filename without * : " << filename_no_star << endl;

                string offset_value_string = Buffer4; 
                int offset_value = atoi(offset_value_string.c_str()); 

                  cout << "offset_value: " << offset_value << endl;

                offset_read(filename_no_star, offset_value);
                lseek_flag = 0;
            }
            else if(list_dir == 1) // if asking
            {
                cout << "make directory" << endl;
                makedirectory();
                Result = sendto(SocketFileDescriptor, contents_output_buffer , 512, 0, (struct sockaddr *)&ClientAddress, ClientLength);
                if(0 > Result)
                    {cout << "ERROR sending to client" << endl;}
                list_dir = 0;
            }
           else
                openandreadfile(filename);
        }

        if(type == 2) //write request
        {
            cout << "THIS IS A WRITE REQUEST!" << endl;

            if(list_dir == 1)
             cout << "request file by an offset" << endl;

            sendfirstack();
        }

        if(type == 3)
        {   
            cout << "This is an DATA type!" << endl;
            creatandwritefile(filename, Buffer, Result);
            ack[3] = 0;
        }


        if(type == 4)
            cout << "This is an ACK Message!" << endl;
        if(type == 5)
            cout << "This is an Error message!" << endl;



        fflush(stdout);

    }

    close(SocketFileDescriptor);
    return 0; 


}