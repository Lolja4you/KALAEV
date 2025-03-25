/*
 * GccApplication1.c
 *
 * Created: 25.03.2025 21:53:39
 *  Author: USER
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <math.h>

#define F_CPU 16000000UL  // ������� ���������� Arduino Nano
#define BAUD 9600         // �������� UART
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)  // ������ �������� ��� �������� UBRR
#define BUFFER_SIZE 64     // ������ ������ ��� ������ ������

// ������������� UART
void UART_Init() {
    // ��������� �������� ��������
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;

    // ��������� ��������� � �����������
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // ��������� ������� �����: 8 ��� ������, 1 ����-���
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// �������� ������ ������� �� UART
void UART_Transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

// �������� ������ �� UART
void UART_SendString(const char* str) {
    while (*str) {
        UART_Transmit(*str++);
    }
}

// ����� ������ �� UART
void UART_ReceiveString(char* buffer, uint8_t buffer_size) {
    uint8_t index = 0;
    char received_char;

    // ������� ������
    memset(buffer, 0, buffer_size);

    // ������ ������ �� ������� ����� ������ ��� ���������� ������
    while (index < buffer_size - 1) {
        // �������� ������ � ������ ���������
        while (!(UCSR0A & (1 << RXC0)));
        received_char = UDR0;

        // ��������� ������� �������� ������� (CR) � ����� ������ (LF)
        if (received_char == '\r' || received_char == '\n') {
            // ���������� ������ ������� ��������
            buffer[index] = '\0';
            return;
        }

        // ���������� ������� � �����
        buffer[index++] = received_char;
    }

    // ���������� ������ ������� ��������, ���� ����� ��������
    buffer[index] = '\0';
}

// ������ 1: ��������� ���� �����
void task_1(char* buffer) {
    uint8_t num1, num2;
    sscanf(buffer, "1 %hhu %hhu", &num1, &num2);
    uint16_t result = num1 * num2;
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "Task 1: %u * %u = %u", num1, num2, result);
    UART_SendString(response);
    UART_SendString("\r\n");
}

// ������ 2: ���������� ������ � �������� ������������� �����
void task_2(char* buffer) {
    uint8_t num;
    sscanf(buffer, "2 %hhu", &num);
    uint8_t count = 0;
    for (uint8_t i = 0; i < 8; i++) {
        if (num & (1 << i)) count++;
    }
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "Task 2: Number of 1s in %u = %u", num, count);
    UART_SendString(response);
    UART_SendString("\r\n");
}

// ������ 3: ������� ���� ����� � �������
void task_3(char* buffer) {
    uint8_t num1, num2;
    sscanf(buffer, "3 %hhu %hhu", &num1, &num2);
    if (num2 == 0) {
        UART_SendString("Task 3: Division by zero is not allowed\r\n");
        return;
    }
    uint8_t quotient = num1 / num2;
    uint8_t remainder = num1 % num2;
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "Task 3: %u / %u = %u, Remainder = %u", num1, num2, quotient, remainder);
    UART_SendString(response);
    UART_SendString("\r\n");
}

// ������ 4: ���������� ����� � �������
void task_4(char* buffer) {
    uint8_t num, power;
    sscanf(buffer, "4 %hhu %hhu", &num, &power);
    uint16_t result = 1;
    for (uint8_t i = 0; i < power; i++) {
        result *= num;
    }
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "Task 4: %u ^ %u = %u", num, power, result);
    UART_SendString(response);
    UART_SendString("\r\n");
}

// ������ 5: ��������� �������� ��������������� ���������
void task_5(char* buffer) {
	// ���������� "5" � ������ ������
	char* expr = buffer + 1;
	char expr_copy[BUFFER_SIZE]; // ������� ����� ��� ���������� ���������
	strncpy(expr_copy, expr, BUFFER_SIZE-1);
	expr_copy[BUFFER_SIZE-1] = '\0';
	
	// ��������� ����� ���������
	if (strlen(expr_copy) > 50) {
		UART_SendString("Error: Expression too long (max 50 chars)\r\n");
		return;
	}
	
	// ��������� ���������� �������
	for (char* c = expr_copy; *c; c++) {
		if (!isdigit(*c) && *c != '+' && *c != '-' && *c != '*' && *c != '/') {
			UART_SendString("Error: Invalid character in expression\r\n");
			return;
		}
	}
	
	// ��������� ��������� �� ����� � ���������
	float numbers[10];
	char operators[9];
	uint8_t num_count = 0;
	uint8_t op_count = 0;
	
	// ��������� �����
	char* token = strtok(expr_copy, "+-*/");
	while (token != NULL && num_count < 10) {
	numbers[num_count++] = atof(token);
	token = strtok(NULL, "+-*/");
	}
	
	// ��������� ��������� �� �������� ������
	for (char* c = expr; *c && op_count < 9; c++) {
	if (*c == '+' || *c == '-' || *c == '*' || *c == '/') {
	operators[op_count++] = *c;
	}
	}
	
	// �������� ������������
	if (num_count != op_count + 1) {
	UART_SendString("Error: Invalid expression format\r\n");
	return;
	}
	
	// ���������� ��������� (� ������ ���������� */)
	for (uint8_t i = 0; i < op_count; i++) {
	if (operators[i] == '*' || operators[i] == '/') {
	float result;
	if (operators[i] == '*') {
	result = numbers[i] * numbers[i+1];
	} else {
	if (numbers[i+1] == 0) {
	UART_SendString("Error: Division by zero\r\n");
	return;
	}
	result = numbers[i] / numbers[i+1];
	}
	
	// ��������� ����� � ���������
	numbers[i] = result;
	for (uint8_t j = i+1; j < num_count-1; j++) {
	numbers[j] = numbers[j+1];
	}
	num_count--;
	
	for (uint8_t j = i; j < op_count-1; j++) {
	operators[j] = operators[j+1];
	}
	op_count--;
	i--; // ��������� ��� ������� �������
	}
	}
	
	// ������ ��������� + � -
	float final_result = numbers[0];
	for (uint8_t i = 0; i < op_count; i++) {
	if (operators[i] == '+') {
	final_result += numbers[i+1];
	} else {
	final_result -= numbers[i+1];
	}
	}
	
	// ������������ � �������� ������
	char response[BUFFER_SIZE];
	snprintf(response, BUFFER_SIZE, "Result: %s = %.2f\r\n", expr, final_result);
	UART_SendString(response);
	}

void choices_task(char* buffer) {
    char task = buffer[0];  // ������ ������ - ����� ������

    switch (task) {
        case '1': task_1(buffer); break;
        case '2': task_2(buffer); break;
        case '3': task_3(buffer); break;
        case '4': task_4(buffer); break;
        case '5': task_5(buffer); break;
        default:
            UART_SendString("Unknown task\r\n");
            break;
    }
}

int main(void) {
    char buffer[BUFFER_SIZE];

    // ������������� UART
    UART_Init();

    while (1) {
        // ����� ������ �� UART
        UART_ReceiveString(buffer, BUFFER_SIZE);

        // ���� ������ ��������, ������������ ������
        if (strlen(buffer) > 0) {
            choices_task(buffer);
        }
    }
}