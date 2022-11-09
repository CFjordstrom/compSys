#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#ifdef __APPLE__
#include "./endian.h"
#else
#include <endian.h>
#endif

#include "./networking.h"
#include "./sha256.h"

char server_ip[IP_LEN];
char server_port[PORT_LEN];
char my_ip[IP_LEN];
char my_port[PORT_LEN];

int c;

/*
 * Gets a sha256 hash of specified data, sourcedata. The hash itself is
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_data_sha(const char* sourcedata, hashdata_t hash, uint32_t data_size, 
    int hash_size)
{
  SHA256_CTX shactx;
  unsigned char shabuffer[hash_size];
  sha256_init(&shactx);
  sha256_update(&shactx, sourcedata, data_size);
  sha256_final(&shactx, shabuffer);

  for (int i=0; i<hash_size; i++)
  {
    hash[i] = shabuffer[i];
  }
}

/*
 * Gets a sha256 hash of specified data file, sourcefile. The hash itself is
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_file_sha(const char* sourcefile, hashdata_t hash, int size)
{
    int casc_file_size;

    FILE* fp = Fopen(sourcefile, "rb");
    if (fp == 0)
    {
        printf("Failed to open source: %s\n", sourcefile);
        return;
    }

    fseek(fp, 0L, SEEK_END);
    casc_file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char buffer[casc_file_size];
    Fread(buffer, casc_file_size, 1, fp);
    Fclose(fp);

    get_data_sha(buffer, hash, casc_file_size, size);
}

int parse_section(char* response_header, int index) {
    char s[4];
    memcpy(s, &response_header[index], 4);
    int* p = (int*)s;
    int r = *p;
    return ntohl(r);
}

/*
 * Combine a password and salt together and hash the result to form the 
 * 'signature'. The result should be written to the 'hash' variable. Note that 
 * as handed out, this function is never called. You will need to decide where 
 * it is sensible to do so.
 */
void get_signature(char* password, char* salt, hashdata_t* hash)
{
    // calculate length of pw and salt, create string to hold concatenation of them
    int len = strlen(password) + strlen(salt);
    char to_hash[len];
    // copy password and hash to string
    strncpy(to_hash, password, strlen(password));
    strncpy(&to_hash[strlen(password)], salt, strlen(salt));

    get_data_sha(to_hash, *hash, len, SHA256_HASH_SIZE);
}

/*
 * Register a new user with a server by sending the username and signature to 
 * the server
 */
void register_user(char* username, char* password, char* salt)
{
    // hash password and salt
    hashdata_t hash;
    get_signature(password, salt, &hash);

    // assemble request header
    char to_send[REQUEST_HEADER_LEN];
    strncpy(to_send, username, USERNAME_LEN);
    memcpy(&to_send[USERNAME_LEN], &hash, SHA256_HASH_SIZE);
    strncpy(&to_send[USERNAME_LEN+SHA256_HASH_SIZE], "", 4);
    
    // open connection
    rio_t rio;
    int client_socket = Open_clientfd(my_ip, my_port);
    Rio_readinitb(&rio, client_socket);

    // create response buffers
    char response_header[RESPONSE_HEADER_LEN];
    char response_body[MAX_PAYLOAD];

    // send request and read response
    Rio_writen(client_socket, to_send, REQUEST_HEADER_LEN);
    Rio_readnb(&rio, response_header, RESPONSE_HEADER_LEN);
    Rio_readnb(&rio, response_body, MAX_PAYLOAD);

    // parse response
    int len = parse_section(response_header, 0);
    int status = parse_section(response_header, 4);
    hashdata_t registration_checksum;
    memcpy(&registration_checksum, &response_header[16], SHA256_HASH_SIZE);
    hashdata_t total_registration_checksum;
    memcpy(&total_registration_checksum, &response_header[16+SHA256_HASH_SIZE], SHA256_HASH_SIZE);

    // if status != ok exit
    if (status != 1) {
        printf("Got unexpected status code: %i\n", status);
        return;
    }

    // if hash of body does not match hash in header exit
    hashdata_t response_hash;
    get_data_sha(response_body, response_hash, len, SHA256_HASH_SIZE);
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
        if (response_hash[i] != registration_checksum[i]) {
            printf("Checksum did not match");
            return;
        }
    }

    get_data_sha(response_body, response_hash, len, SHA256_HASH_SIZE);
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
        if (response_hash[i] != total_registration_checksum[i]) {
            printf("Checksum did not match");
            return;
        }
    }

    Fputs(response_body, stdout);
    printf("\n");
}

/*
 * Get a file from the server by sending the username and signature, along with
 * a file path. Note that this function should be able to deal with both small 
 * and large files. 
 */
void get_file(char* username, char* password, char* salt, char* to_get)
{
    // hash password and salt
    hashdata_t hash;
    get_signature(password, salt, &hash);

    // length of filepath in host and network byte order
    int path_len = strlen(to_get);
    int n_path_len = htonl(path_len);

    // assemble request header
    char request_header[REQUEST_HEADER_LEN+path_len];
    strncpy(request_header, username, USERNAME_LEN);
    memcpy(&request_header[USERNAME_LEN], &hash, SHA256_HASH_SIZE);
    memcpy(&request_header[USERNAME_LEN+SHA256_HASH_SIZE], &n_path_len, 4);
    strncpy(&request_header[REQUEST_HEADER_LEN], to_get, path_len);

    // open connection
    rio_t rio;
    int client_socket = Open_clientfd(my_ip, my_port);
    Rio_readinitb(&rio, client_socket);
    char response_header[RESPONSE_HEADER_LEN];
    
    // send request and read response header
    Rio_writen(client_socket, request_header, REQUEST_HEADER_LEN+path_len);
    Rio_readnb(&rio, response_header, RESPONSE_HEADER_LEN);

    // parse first 16 bytes from response header
    int len = parse_section(response_header, 0);
    int status = parse_section(response_header, 4);
    int block_num = parse_section(response_header, 8);
    int num_blocks = parse_section(response_header, 12);

    // parse next 64 bytes from response header
    hashdata_t block_checksum;
    memcpy(&block_checksum, &response_header[16], SHA256_HASH_SIZE);
    hashdata_t total_checksum;
    memcpy(&total_checksum, &response_header[16+SHA256_HASH_SIZE], SHA256_HASH_SIZE);

    // read first response into response_body
    char response_body[len];
    Rio_readnb(&rio, response_body, len);
    printf("Block %i (%i/%i)\n", block_num, 1, num_blocks);

    // if status != ok exit
    if (status != 1) {
        printf("Got unexpected status code: %i\n", status);
        return;
    }

    // if hash of body does not match hash in header exit
    hashdata_t response_hash;
    get_data_sha(response_body, response_hash, len, SHA256_HASH_SIZE);
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
        if (response_hash[i] != block_checksum[i]) {
            printf("Block %i/%i checksums do not match\n", block_num, num_blocks);
            return;
        }
    }

    // if amount of blocks to be read is 0, check if total hash matches,
    // if it does, write response to a file and stop
    if (num_blocks == 1) {
        hashdata_t total_hash;
        get_data_sha(response_body, total_hash, len, SHA256_HASH_SIZE);
        for (int i = 0; i < SHA256_HASH_SIZE; i++) {
            if (total_hash[i] != total_checksum[i]) {
                printf("Block %i/%i checksums do not match\n", block_num, num_blocks);
                return;
            }
        }
        FILE* fp = Fopen(to_get, "w");
        Fwrite(response_body, 1, strlen(response_body), fp);
        printf("Retrieved data written to %s\n", to_get);
        Fclose(fp);
        return;
    }

    // there are more than one block, create array of strings to hold result from
    // all blocks, and copy first block into the array
    int blocks_read = 1;
    char all_blocks[num_blocks][MAX_PAYLOAD];
    int reading = 1;
    memcpy(all_blocks[block_num], response_body, len);

    while(reading) {
        // read header for new response, parse it like before and check codes etc.
        Rio_readnb(&rio, response_header, RESPONSE_HEADER_LEN);

        int len = parse_section(response_header, 0);
        int status = parse_section(response_header, 4);
        int block_num = parse_section(response_header, 8);
        int num_blocks = parse_section(response_header, 12);

        hashdata_t block_checksum;
        memcpy(&block_checksum, &response_header[16], SHA256_HASH_SIZE);
        hashdata_t total_checksum;
        memcpy(&total_checksum, &response_header[16+SHA256_HASH_SIZE], SHA256_HASH_SIZE);

        char response_body[len];
        Rio_readnb(&rio, response_body, len);
        blocks_read++;
        printf("Block %i (%i/%i)\n", block_num, blocks_read, num_blocks);

        if (status != 1) {
            printf("Got unexpected status code: %i\n", status);
            return;
        }

        hashdata_t response_hash;
        get_data_sha(response_body, response_hash, len, SHA256_HASH_SIZE);
        for (int i = 0; i < SHA256_HASH_SIZE; i++) {
            if (response_hash[i] != block_checksum[i]) {
                printf("Block %i/%i checksums do not match\n", block_num, num_blocks);
                return;
            }
        }
        // copy response into array holding all blocks
        memcpy(all_blocks[block_num], response_body, len);
        // if amount of blocks read = amount of blocks to be read, stop reading
        if (blocks_read == num_blocks) {
            reading = 0;
        }
    }

    // check if hash of all blocks matches total hash from header
    hashdata_t total_hash;
    get_data_sha(all_blocks[0], total_hash, strlen(all_blocks[0]), SHA256_HASH_SIZE);
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
        if (total_hash[i] != total_checksum[i]) {
            printf("Checksum for all blocks does not match\n");
            return;
        }
    }

    // data is correct, so write it to file
    FILE* fp = Fopen(to_get, "w");
    Fwrite(all_blocks[0], 1, strlen(all_blocks[0]), fp);
    printf("Retrieved data written to %s\n", to_get);
    Fclose(fp);
}

int main(int argc, char **argv)
{
    // Users should call this script with a single argument describing what 
    // config to use
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Read in configuration options. Should include a client_directory, 
    // client_ip, client_port, server_ip, and server_port
    char buffer[128];
    fprintf(stderr, "Got config path at: %s\n", argv[1]);
    FILE* fp = Fopen(argv[1], "r");
    while (fgets(buffer, 128, fp)) {
        if (starts_with(buffer, CLIENT_IP)) {
            memcpy(my_ip, &buffer[strlen(CLIENT_IP)], 
                strcspn(buffer, "\r\n")-strlen(CLIENT_IP));
            if (!is_valid_ip(my_ip)) {
                fprintf(stderr, ">> Invalid client IP: %s\n", my_ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, CLIENT_PORT)) {
            memcpy(my_port, &buffer[strlen(CLIENT_PORT)], 
                strcspn(buffer, "\r\n")-strlen(CLIENT_PORT));
            if (!is_valid_port(my_port)) {
                fprintf(stderr, ">> Invalid client port: %s\n", my_port);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, SERVER_IP)) {
            memcpy(server_ip, &buffer[strlen(SERVER_IP)], 
                strcspn(buffer, "\r\n")-strlen(SERVER_IP));
            if (!is_valid_ip(server_ip)) {
                fprintf(stderr, ">> Invalid server IP: %s\n", server_ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, SERVER_PORT)) {
            memcpy(server_port, &buffer[strlen(SERVER_PORT)], 
                strcspn(buffer, "\r\n")-strlen(SERVER_PORT));
            if (!is_valid_port(server_port)) {
                fprintf(stderr, ">> Invalid server port: %s\n", server_port);
                exit(EXIT_FAILURE);
            }
        }        
    }
    fclose(fp);

    fprintf(stdout, "Client at: %s:%s\n", my_ip, my_port);
    fprintf(stdout, "Server at: %s:%s\n", server_ip, server_port);

    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
    char user_salt[SALT_LEN+1];
    
    fprintf(stdout, "Enter a username to proceed: ");
    scanf("%16s", username);
    while ((c = getchar()) != '\n' && c != EOF);
    // Clean up username string as otherwise some extra chars can sneak in.
    for (int i=strlen(username); i<USERNAME_LEN; i++)
    {
        username[i] = '\0';
    }
 
    fprintf(stdout, "Enter your password to proceed: ");
    scanf("%16s", password);
    while ((c = getchar()) != '\n' && c != EOF);
    // Clean up password string as otherwise some extra chars can sneak in.
    for (int i=strlen(password); i<PASSWORD_LEN; i++)
    {
        password[i] = '\0';
    }

    // Note that a random salt should be used, but you may find it easier to
    // repeatedly test the same user credentials by using the hard coded value
    // below instead, and commenting out this randomly generating section.
    /*for (int i=0; i<SALT_LEN; i++)
    {
        user_salt[i] = 'a' + (random() % 26);
    }
    user_salt[SALT_LEN] = '\0';*/
    strncpy(user_salt, 
        "0123456789012345678901234567890123456789012345678901234567890123\0", 
        SALT_LEN+1);

    fprintf(stdout, "Using salt: %s\n", user_salt);

    // The following function calls have been added as a structure to a 
    // potential solution demonstrating the core functionality. Feel free to 
    // add, remove or otherwise edit. 

    // Register the given user
    register_user(username, password, user_salt);

    // Retrieve the smaller file, that doesn't not require support for blocks
    get_file(username, password, user_salt, "tiny.txt");

    // Retrieve the larger file, that requires support for blocked messages
    get_file(username, password, user_salt, "hamlet.txt");

    exit(EXIT_SUCCESS);
}