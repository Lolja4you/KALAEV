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

#define F_CPU 16000000UL  // Частота процессора Arduino Nano
#define BAUD 9600         // Скорость UART
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)  // Расчет значения для регистра UBRR
#define BUFFER_SIZE 64     // Размер буфера для приема строки

// Инициализация UART
void UART_Init() {
    // Установка скорости передачи
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;

    // Включение приемника и передатчика
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // Установка формата кадра: 8 бит данных, 1 стоп-бит
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Отправка одного символа по UART
void UART_Transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

// Отправка строки по UART
void UART_SendString(const char* str) {
    while (*str) {
        UART_Transmit(*str++);
    }
}

// Прием строки из UART
void UART_ReceiveString(char* buffer, uint8_t buffer_size) {
    uint8_t index = 0;
    char received_char;

    // Очистка буфера
    memset(buffer, 0, buffer_size);

    // Чтение данных до символа новой строки или заполнения буфера
    while (index < buffer_size - 1) {
        // Ожидание данных в буфере приемника
        while (!(UCSR0A & (1 << RXC0)));
        received_char = UDR0;

        // Обработка символа возврата каретки (CR) и новой строки (LF)
        if (received_char == '\r' || received_char == '\n') {
            // Завершение строки нулевым символом
            buffer[index] = '\0';
            return;
        }

        // Сохранение символа в буфер
        buffer[index++] = received_char;
    }

    // Завершение строки нулевым символом, если буфер заполнен
    buffer[index] = '\0';
}

// Задача 1: Умножение двух чисел
void task_1(char* buffer) {
    uint8_t num1, num2;
    sscanf(buffer, "1 %hhu %hhu", &num1, &num2);
    uint16_t result = num1 * num2;
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "Task 1: %u * %u = %u", num1, num2, result);
    UART_SendString(response);
    UART_SendString("\r\n");
}

// Задача 2: Количество единиц в бинарном представлении числа
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

// Задача 3: Деление двух чисел и остаток
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

// Задача 4: Возведение числа в степень
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

// Задача 5: Обработка сложного математического выражения
void task_5(char* buffer) {
	// Пропускаем "5" в начале строки
	char* expr = buffer + 1;
	char expr_copy[BUFFER_SIZE]; // Создаем копию для безопасной обработки
	strncpy(expr_copy, expr, BUFFER_SIZE-1);
	expr_copy[BUFFER_SIZE-1] = '\0';
	
	// Проверяем длину выражения
	if (strlen(expr_copy) > 50) {
		UART_SendString("Error: Expression too long (max 50 chars)\r\n");
		return;
	}
	
	// Проверяем допустимые символы
	for (char* c = expr_copy; *c; c++) {
		if (!isdigit(*c) && *c != '+' && *c != '-' && *c != '*' && *c != '/') {
			UART_SendString("Error: Invalid character in expression\r\n");
			return;
		}
	}
	
	// Разбиваем выражение на числа и операторы
	float numbers[10];
	char operators[9];
	uint8_t num_count = 0;
	uint8_t op_count = 0;
	
	// Извлекаем числа
	char* token = strtok(expr_copy, "+-*/");
	while (token != NULL && num_count < 10) {
	numbers[num_count++] = atof(token);
	token = strtok(NULL, "+-*/");
	}
	
	// Извлекаем операторы из исходной строки
	for (char* c = expr; *c && op_count < 9; c++) {
	if (*c == '+' || *c == '-' || *c == '*' || *c == '/') {
	operators[op_count++] = *c;
	}
	}
	
	// Проверка корректности
	if (num_count != op_count + 1) {
	UART_SendString("Error: Invalid expression format\r\n");
	return;
	}
	
	// Вычисление выражения (с учетом приоритета */)
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
	
	// Обновляем числа и операторы
	numbers[i] = result;
	for (uint8_t j = i+1; j < num_count-1; j++) {
	numbers[j] = numbers[j+1];
	}
	num_count--;
	
	for (uint8_t j = i; j < op_count-1; j++) {
	operators[j] = operators[j+1];
	}
	op_count--;
	i--; // Повторяем для текущей позиции
	}
	}
	
	// Теперь вычисляем + и -
	float final_result = numbers[0];
	for (uint8_t i = 0; i < op_count; i++) {
	if (operators[i] == '+') {
	final_result += numbers[i+1];
	} else {
	final_result -= numbers[i+1];
	}
	}
	
	// Формирование и отправка ответа
	char response[BUFFER_SIZE];
	snprintf(response, BUFFER_SIZE, "Result: %s = %.2f\r\n", expr, final_result);
	UART_SendString(response);
	}

void choices_task(char* buffer) {
    char task = buffer[0];  // Первый символ - номер задачи

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

    // Инициализация UART
    UART_Init();

    while (1) {
        // Прием строки из UART
        UART_ReceiveString(buffer, BUFFER_SIZE);

        // Если данные получены, обрабатываем задачу
        if (strlen(buffer) > 0) {
            choices_task(buffer);
        }
    }
}