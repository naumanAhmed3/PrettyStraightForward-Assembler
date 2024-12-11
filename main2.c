#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>


#define MAX_LINES 1000
#define MAX_LINE_LEN 256
#define MAX_LABEL_LEN 30

typedef struct {
    char instruction[MAX_LINE_LEN];
} Line;

typedef struct {
    char label[MAX_LABEL_LEN];
    int address;
} Label;

Line lines[MAX_LINES];
int line_count = 0;
Label symbol_table[MAX_LINES];
int label_count = 0;

// Function prototypes
void read_input_file(const char *filename);
void parse_labels();
int get_register_number(const char *reg);
int get_label_address(const char *label);
unsigned int translate_instruction(const char *instr, int current_address);
void output_machine_code();
char* trim_whitespace(char *str);

int main(int argc, char *argv[]) {

    clock_t start_time, end_time;
    double read_time, parse_time, translate_time, total_time;

// Start overall timing
    start_time = clock();

    if (argc != 2) {
        printf("Usage: %s inputFile\n", argv[0]);
        return 1;
    }

    clock_t read_start = clock();
    read_input_file(argv[1]); // Existing function
    clock_t read_end = clock();
read_time = (double)(read_end - read_start) / CLOCKS_PER_SEC;

    
   clock_t parse_start = clock();
parse_labels(); // Existing function
clock_t parse_end = clock();
parse_time = (double)(parse_end - parse_start) / CLOCKS_PER_SEC;

   clock_t translate_start = clock();
output_machine_code(); // Existing function
clock_t translate_end = clock();
translate_time = (double)(translate_end - translate_start) / CLOCKS_PER_SEC;


    end_time = clock();
total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

// Print performance metrics
printf("\nPerformance Metrics:\n");
printf("Time taken to read input file: %.6f seconds\n", read_time);
printf("Time taken to parse labels: %.6f seconds\n", parse_time);
printf("Time taken to translate instructions: %.6f seconds\n", translate_time);
printf("Total execution time: %.6f seconds\n", total_time);

return 0;

}

void read_input_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    printf("Assembly language program:\n");
    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, sizeof(buffer), file)) {
        // Remove comments and trim whitespace
        char *comment = strchr(buffer, '#');
        if (comment) *comment = '\0';
        char *line = trim_whitespace(buffer);

        if (*line == '\0') continue; // Skip empty lines

        // Split the line into individual instructions based on semicolon delimiter
        char *instr = strtok(line, ";");
        while (instr != NULL && line_count < MAX_LINES) {
            // Trim leading and trailing whitespace
            char *trimmed_instr = trim_whitespace(instr);
            if (strlen(trimmed_instr) > 0) {
                // Store the instruction
                strcpy(lines[line_count].instruction, trimmed_instr);
                printf("%s\n", trimmed_instr);
                line_count++;
            }
            instr = strtok(NULL, ";");
        }
    }

    fclose(file);
}

void parse_labels() {
    int address = 0x00400000;
    for (int i = 0; i < line_count; ) {
        char *line = lines[i].instruction;

        // Check if the line contains a label
        char *colon = strchr(line, ':');
        if (colon) {
            // Extract label
            int label_len = colon - line;
            char label[MAX_LABEL_LEN];
            strncpy(label, line, label_len);
            label[label_len] = '\0';
            // Trim any whitespace from label
            char *trimmed_label = trim_whitespace(label);

            // Add label to symbol table
            strcpy(symbol_table[label_count].label, trimmed_label);
            symbol_table[label_count].address = address;
            label_count++;

            // Remove label from instruction
            char *remaining_instr = colon + 1;
            remaining_instr = trim_whitespace(remaining_instr);
            if (*remaining_instr == '\0') {
                // No instruction on this line, remove it
                for (int j = i; j < line_count -1; j++) {
                    strcpy(lines[j].instruction, lines[j+1].instruction);
                }
                line_count--;
                // Do not increment i to check the new line at current index
                continue;
            } else {
                // Replace the line's instruction with the remaining instruction
                strcpy(lines[i].instruction, remaining_instr);
            }
        }

        address += 4;
        i++;
    }
}

int get_register_number(const char *reg) {
    // Define a struct for register mappings
    typedef struct {
        char *name;
        int number;
    } Register;

    Register registers[] = {
        {"$zero", 0}, {"$0", 0},
        {"$at", 1}, {"$1", 1},
        {"$v0", 2}, {"$2", 2},
        {"$v1", 3}, {"$3", 3},
        {"$a0", 4}, {"$4", 4},
        {"$a1", 5}, {"$5", 5},
        {"$a2", 6}, {"$6", 6},
        {"$a3", 7}, {"$7", 7},
        {"$t0", 8}, {"$8", 8},
        {"$t1", 9}, {"$9", 9},
        {"$t2", 10}, {"$10", 10},
        {"$t3", 11}, {"$11", 11},
        {"$t4", 12}, {"$12", 12},
        {"$t5", 13}, {"$13", 13},
        {"$t6", 14}, {"$14", 14},
        {"$t7", 15}, {"$15", 15},
        {"$s0", 16}, {"$16", 16},
        {"$s1", 17}, {"$17", 17},
        {"$s2", 18}, {"$18", 18},
        {"$s3", 19}, {"$19", 19},
        {"$s4", 20}, {"$20", 20},
        {"$s5", 21}, {"$21", 21},
        {"$s6", 22}, {"$22", 22},
        {"$s7", 23}, {"$23", 23},
        {"$t8", 24}, {"$24", 24},
        {"$t9", 25}, {"$25", 25},
        {"$k0", 26}, {"$26", 26},
        {"$k1", 27}, {"$27", 27},
        {"$gp", 28}, {"$28", 28},
        {"$sp", 29}, {"$29", 29},
        {"$fp", 30}, {"$30", 30},
        {"$ra", 31}, {"$31", 31},
        {NULL, -1}
    };

    for (int i =0; registers[i].name != NULL; i++) {
        if (strcmp(reg, registers[i].name) ==0 ) {
            return registers[i].number;
        }
    }

    return -1; // Error: Unknown register
}

int get_label_address(const char *label) {
    for (int i =0; i < label_count; i++) {
        if (strcmp(symbol_table[i].label, label)==0) {
            return symbol_table[i].address;
        }
    }
    return -1; // Error: Label not found
}

unsigned int translate_instruction(const char *instr, int current_address) {
    char opcode_str[10];
    char operands[4][30]; // Increased size for label operands
    int num_operands =0;

    // Tokenize the instruction
    char *token;
    char instr_copy[MAX_LINE_LEN];
    strcpy(instr_copy, instr);
    token = strtok(instr_copy, " ,()\n\t");
    if (!token) return 0;

    strcpy(opcode_str, token);

    // Read operands
    while ((token = strtok(NULL, " ,()\n\t")) != NULL && num_operands <4) {
        strcpy(operands[num_operands++], token);
    }

    // Map opcode to operation
    if (strcmp(opcode_str, "add") == 0) {
        // R-type: add $rd, $rs, $rt
        if (num_operands !=3) {
            printf("Error: Incorrect number of operands for 'add'\n");
            exit(EXIT_FAILURE);
        }
        int rd = get_register_number(operands[0]);
        int rs = get_register_number(operands[1]);
        int rt = get_register_number(operands[2]);
        if (rd ==-1 || rs ==-1 || rt==-1) {
            printf("Error: Invalid register in 'add'\n");
            exit(EXIT_FAILURE);
        }
        unsigned int machine_code = (0 <<26) | (rs<<21) | (rt<<16) | (rd<<11) | (0<<6) | 0x20;
        return machine_code;
    }
    else if (strcmp(opcode_str, "sub") ==0) {
        // R-type: sub $rd, $rs, $rt
        if (num_operands !=3) {
            printf("Error: Incorrect number of operands for 'sub'\n");
            exit(EXIT_FAILURE);
        }
        int rd = get_register_number(operands[0]);
        int rs = get_register_number(operands[1]);
        int rt = get_register_number(operands[2]);
        if (rd ==-1 || rs ==-1 || rt==-1) {
            printf("Error: Invalid register in 'sub'\n");
            exit(EXIT_FAILURE);
        }
        unsigned int machine_code = (0 <<26) | (rs<<21) | (rt<<16) | (rd<<11) | (0<<6) | 0x22;
        return machine_code;
    }
    else if (strcmp(opcode_str, "and") ==0) {
        // R-type: and $rd, $rs, $rt
        if (num_operands !=3) {
            printf("Error: Incorrect number of operands for 'and'\n");
            exit(EXIT_FAILURE);
        }
        int rd = get_register_number(operands[0]);
        int rs = get_register_number(operands[1]);
        int rt = get_register_number(operands[2]);
        if (rd ==-1 || rs ==-1 || rt==-1) {
            printf("Error: Invalid register in 'and'\n");
            exit(EXIT_FAILURE);
        }
        unsigned int machine_code = (0 <<26) | (rs<<21) | (rt<<16) | (rd<<11) | (0<<6) | 0x24;
        return machine_code;
    }
    else if (strcmp(opcode_str, "or") ==0) {
        // R-type: or $rd, $rs, $rt
        if (num_operands !=3) {
            printf("Error: Incorrect number of operands for 'or'\n");
            exit(EXIT_FAILURE);
        }
        int rd = get_register_number(operands[0]);
        int rs = get_register_number(operands[1]);
        int rt = get_register_number(operands[2]);
        if (rd ==-1 || rs ==-1 || rt==-1) {
            printf("Error: Invalid register in 'or'\n");
            exit(EXIT_FAILURE);
        }
        unsigned int machine_code = (0 <<26) | (rs<<21) | (rt<<16) | (rd<<11) | (0<<6) | 0x25;
        return machine_code;
    }
    else if (strcmp(opcode_str, "xor") ==0) {
        // R-type: xor $rd, $rs, $rt
        if (num_operands !=3) {
            printf("Error: Incorrect number of operands for 'xor'\n");
            exit(EXIT_FAILURE);
        }
        int rd = get_register_number(operands[0]);
        int rs = get_register_number(operands[1]);
        int rt = get_register_number(operands[2]);
        if (rd ==-1 || rs ==-1 || rt==-1) {
            printf("Error: Invalid register in 'xor'\n");
            exit(EXIT_FAILURE);
        }
        unsigned int machine_code = (0 <<26) | (rs<<21) | (rt<<16) | (rd<<11) | (0<<6) | 0x26;
        return machine_code;
    }
    else if (strcmp(opcode_str, "slt") ==0) {
        // R-type: slt $rd, $rs, $rt
        if (num_operands !=3) {
            printf("Error: Incorrect number of operands for 'slt'\n");
            exit(EXIT_FAILURE);
        }
        int rd = get_register_number(operands[0]);
        int rs = get_register_number(operands[1]);
        int rt = get_register_number(operands[2]);
        if (rd ==-1 || rs ==-1 || rt==-1) {
            printf("Error: Invalid register in 'slt'\n");
            exit(EXIT_FAILURE);
        }
        unsigned int machine_code = (0 <<26) | (rs<<21) | (rt<<16) | (rd<<11) | (0<<6) | 0x2A;
        return machine_code;
    }
    else if (strcmp(opcode_str, "addi") ==0) {
        // I-type: addi $rt, $rs, immediate
        if (num_operands !=3) {
            printf("Error: Incorrect number of operands for 'addi'\n");
            exit(EXIT_FAILURE);
        }
        int rt = get_register_number(operands[0]);
        int rs = get_register_number(operands[1]);
        int immediate = atoi(operands[2]);
        if (rt ==-1 || rs ==-1) {
            printf("Error: Invalid register in 'addi'\n");
            exit(EXIT_FAILURE);
        }
        unsigned int machine_code = (0x08 <<26) | (rs<<21) | (rt<<16) | (immediate &0xFFFF);
        return machine_code;
    }
    else if (strcmp(opcode_str, "andi") ==0) {
        // I-type: andi $rt, $rs, immediate
        if (num_operands !=3) {
            printf("Error: Incorrect number of operands for 'andi'\n");
            exit(EXIT_FAILURE);
        }
        int rt = get_register_number(operands[0]);
        int rs = get_register_number(operands[1]);
        int immediate = atoi(operands[2]);
        if (rt ==-1 || rs ==-1) {
            printf("Error: Invalid register in 'andi'\n");
            exit(EXIT_FAILURE);
        }
        unsigned int machine_code = (0x0C <<26) | (rs<<21) | (rt<<16) | (immediate &0xFFFF);
        return machine_code;
    }
    else if (strcmp(opcode_str, "ori") ==0) {
        // I-type: ori $rt, $rs, immediate
        if (num_operands !=3) {
            printf("Error: Incorrect number of operands for 'ori'\n");
            exit(EXIT_FAILURE);
        }
        int rt = get_register_number(operands[0]);
        int rs = get_register_number(operands[1]);
        int immediate = atoi(operands[2]);
        if (rt ==-1 || rs ==-1) {
            printf("Error: Invalid register in 'ori'\n");
            exit(EXIT_FAILURE);
        }
        unsigned int machine_code = (0x0D <<26) | (rs<<21) | (rt<<16) | (immediate &0xFFFF);
        return machine_code;
    }
    else if (strcmp(opcode_str, "lw") ==0) {
        // I-type: lw $rt, offset($rs) OR lw $rt, ($rs)
        if (num_operands <2 || num_operands >3) {
            printf("Error: Incorrect number of operands for 'lw'\n");
            exit(EXIT_FAILURE);
        }
        int rt = get_register_number(operands[0]);
        int rs;
        int offset =0; // Default offset

        if (num_operands ==2) {
            // Expecting lw $rt, ($rs)
            if (operands[1][0] != '$') {
                printf("Error: Incorrect format for 'lw'\n");
                exit(EXIT_FAILURE);
            }
            rs = get_register_number(operands[1]);
            if (rs ==-1) {
                printf("Error: Invalid register in 'lw'\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (num_operands ==3) {
            // Expecting lw $rt, offset($rs)
            offset = atoi(operands[1]);
            rs = get_register_number(operands[2]);
            if (rs ==-1) {
                printf("Error: Invalid register in 'lw'\n");
                exit(EXIT_FAILURE);
            }
        }

        unsigned int machine_code = (0x23 <<26) | (rs<<21) | (rt<<16) | (offset &0xFFFF);
        return machine_code;
    }
    else if (strcmp(opcode_str, "sw") ==0) {
        // I-type: sw $rt, offset($rs) OR sw $rt, ($rs)
        if (num_operands <2 || num_operands >3) {
            printf("Error: Incorrect number of operands for 'sw'\n");
            exit(EXIT_FAILURE);
        }
        int rt = get_register_number(operands[0]);
        int rs;
        int offset =0; // Default offset

        if (num_operands ==2) {
            // Expecting sw $rt, ($rs)
            if (operands[1][0] != '$') {
                printf("Error: Incorrect format for 'sw'\n");
                exit(EXIT_FAILURE);
            }
            rs = get_register_number(operands[1]);
            if (rs ==-1) {
                printf("Error: Invalid register in 'sw'\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (num_operands ==3) {
            // Expecting sw $rt, offset($rs)
            offset = atoi(operands[1]);
            rs = get_register_number(operands[2]);
            if (rs ==-1) {
                printf("Error: Invalid register in 'sw'\n");
                exit(EXIT_FAILURE);
            }
        }

        unsigned int machine_code = (0x2B <<26) | (rs<<21) | (rt<<16) | (offset &0xFFFF);
        return machine_code;
    }
    else if (strcmp(opcode_str, "beq") ==0) {
        // I-type: beq $rs, $rt, label or immediate offset
        if (num_operands !=3) {
            printf("Error: Incorrect number of operands for 'beq'\n");
            exit(EXIT_FAILURE);
        }
        int rs = get_register_number(operands[0]);
        int rt = get_register_number(operands[1]);
        if (rs ==-1 || rt ==-1) {
            printf("Error: Invalid register in 'beq'\n");
            exit(EXIT_FAILURE);
        }
        int offset;
        if (isalpha(operands[2][0])) {
            // Operand is a label
            int label_address = get_label_address(operands[2]);
            if (label_address ==-1) {
                printf("Error: Undefined label '%s'\n", operands[2]);
                exit(EXIT_FAILURE);
            }
            offset = (label_address - (current_address +4)) /4;
        }
        else {
            // Operand is a numeric immediate offset
            offset = atoi(operands[2]);
        }
        unsigned int machine_code = (0x04 <<26) | (rs<<21) | (rt<<16) | (offset &0xFFFF);
        return machine_code;
    }
    else if (strcmp(opcode_str, "j") ==0) {
        // J-type: j label or immediate address
        if (num_operands !=1) {
            printf("Error: Incorrect number of operands for 'j'\n");
            exit(EXIT_FAILURE);
        }
        unsigned int address_field;
        if (isalpha(operands[0][0])) {
            // Operand is a label
            int label_address = get_label_address(operands[0]);
            if (label_address ==-1) {
                printf("Error: Undefined label '%s'\n", operands[0]);
                exit(EXIT_FAILURE);
            }
            address_field = (label_address >>2) &0x03FFFFFF;
        }
        else {
            // Operand is an immediate address
            unsigned int immediate_address = (unsigned int)strtoul(operands[0], NULL,0);
            address_field = (immediate_address >>2) &0x03FFFFFF;
        }
        unsigned int machine_code = (0x02 <<26) | address_field;
        return machine_code;
    }
    else {
        printf("Error: Unknown instruction '%s'\n", opcode_str);
        exit(EXIT_FAILURE);
    }
}

void output_machine_code() {
    int address = 0x00400000;
    printf("\nMachine Code:\n");
    for (int i=0; i < line_count; i++) {
        if (strlen(lines[i].instruction)==0) {
            address +=4;
            continue; // Skip empty instructions (labels only)
        }
        unsigned int machine_code = translate_instruction(lines[i].instruction, address);
        printf("0x%08X 0x%08X %s\n", address, machine_code, lines[i].instruction);
        address +=4;
    }
}

char* trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str ==0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) -1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end+1) = '\0';

    return str;
}
