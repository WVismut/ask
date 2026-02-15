#include <stdio.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// function from my other project
// it compares two strings, but first one will be converted to lowercase
bool custom_compare(char *first_operand, char *second_operand) {
    char res[128];
    int i;
    // first operand will be converted to lowercase
    for (i = 0; i < strlen(first_operand); i++) {
        res[i] = tolower(first_operand[i]);
    }

    res[i] = 0;

    if (strcmp(res, second_operand) == 0) {
        return true;
    } else {
        return false;
    }
}

int main(int argc, char *argv[]) {

    // if no arguments provided
    if (argc == 1) {
        printf("You should provide at least one argumnet!\n"
               "Here are a few examples, of how you should use this command\n"
               "ask How to see contant of a text file?\n"
               "ask How to update my system?\n");
        return 1;
    }

    FILE *commands_file = fopen("commands.json", "r");
    if (commands_file == NULL) {
        printf("Error opening file!\n"
               "(commands_file pointer is equals to null)\n"
               "Check is there actually a file called \"commands.json\" in this directory\n");
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
               "(json pointer is equals to null)\n"
               "There must be an error in the \"commands.json file\"");
        cJSON_Delete(json);
        return 1;
    }

    // chech, if the *json is an array
    if (!cJSON_IsArray(json)) {
        printf("JSON parsing error!\n"
               "(expexted *json to be array)\n"
               "There must be an error in the \"commands.json file\"");
        cJSON_Delete(json);
        return 1;
    }

    // variables
    cJSON *command = NULL;
    cJSON *command_name = NULL;
    cJSON *command_tags = NULL;
    cJSON *best_candidate = NULL;
    cJSON *tag = NULL;
    
    int tag_index;
    int best_score = 0;
    int current_score;
    int commands_index = 0;

    // search loop
    while (true) {
        // reset current score
        current_score = 0;

        // get command
        command = cJSON_GetArrayItem(json, commands_index);
        command_name = cJSON_GetObjectItem(command, "name");
        command_tags = cJSON_GetObjectItem(command, "tags");

        // get the score of the command
        tag_index = 0;
        tag = cJSON_GetArrayItem(command_tags, tag_index);
        while (tag->next != NULL) {
            for (int i = 1; i < argc; i++) {
                if (strcmp(tag->valuestring, argv[i]) == 0)
                    current_score++;
            }

            tag_index++;
            tag = cJSON_GetArrayItem(command_tags, tag_index);
        }
        
        if (current_score > best_score) {
            best_score = current_score;
            best_candidate = command;
        }

        // if there are no more commands
        if (command->next == NULL)
            break;

        // update the value of *command var
        commands_index++;
    }

    printf("Best score: %d\n", best_score);
    printf("Best candidate: %s\n", cJSON_GetObjectItem(best_candidate, "name")->valuestring);

    // end of the program
    cJSON_Delete(json);
    return 0;
}