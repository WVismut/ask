#include <stdio.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char argv[]) {

    // if no arguments provided
    if (argc == 1) {
        printf("You should provide at least one argumnet!\n"
               "Here are a few examples, of how you should use this command\n"
               "ask How to see contant of a text file?\n"
               "ask \"How to update my system?\"\n");
        return 1;
    }

    FILE *commands_file = fopen("commands.json", "r");
    if (commands_file == NULL) {
        printf("Error opening file!\n"
               "( commads_file == NULL )\n"
               "Check is there actually a file called \"commands.json\" in this directory/folder\n");
        return 1;
    }

    // get file lenth
    fseek(commands_file, 0, SEEK_END);
    long lenth = ftell(commands_file);
    fseek(commands_file, 0, SEEK_SET);

    // load commands.json file in memory
    char *json_string = malloc(lenth + 1);
    fread(json_string, 1, lenth, commands_file);
    json_string[lenth] = '\0';
    fclose(commands_file);

    // parse json
    cJSON *json = cJSON_Parse(json_string);
    free(json_string);

    if (json == NULL) {
        printf("JSON parsing error!\n"
               "There must be an error in the \"commands.json file\"");
        return 1;
    }

    // chech, if the *json is an array
    if (!cJSON_IsArray(json)) {
        printf("JSON parsing error!\n"
               "There must be an error in the \"commands.json file\"");
        cJSON_Delete(json);
        return 1;
    }

    // variables
    cJSON *command_name = NULL;
    cJSON *command = NULL;
    cJSON *command_tags = NULL;
    cJSON *args_tags = NULL;
    int i = 0;

    // search loop
    while (true) {
        // get command
        command = cJSON_GetArrayItem(json, i);
        command_name = cJSON_GetObjectItem(command, "name");
        printf("Commad name: %s\n", command_name->valuestring);

        // if there are no more commands
        if (command->next == NULL)
            break;

        // update the value of *command var
        i++;
        command = cJSON_GetArrayItem(json, i);
    }

    // end of the program
    cJSON_Delete(json);
    return 0;
}