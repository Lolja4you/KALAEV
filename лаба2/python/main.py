import serial
import serial.tools.list_ports
import time
import sys

def find_arduino_port():
    """Автоматически находит порт Arduino с чипом CH340"""
    arduino_ports = [
        p.device
        for p in serial.tools.list_ports.comports()
        if 'CH340' in p.description or  # Для китайских клонов
           'Arduino' in p.description or  # Для оригинальных Arduino
           (p.vid == 0x1A86 and p.pid == 0x7523)  # VID/PID для CH340
    ]
    
    if not arduino_ports:
        raise serial.SerialException("Arduino с CH340 не найдена")
    if len(arduino_ports) > 1:
        print("Найдено несколько устройств:", arduino_ports)
    return arduino_ports[0]

def connect_to_arduino():
    """Устанавливает соединение с Arduino"""
    max_attempts = 5
    for attempt in range(max_attempts):
        try:
            port = find_arduino_port()
            ser = serial.Serial(port, 9600, timeout=1)
            print(f"Подключено к {port}")
            time.sleep(2)  # Даем время Arduino на инициализацию
            return ser
        except serial.SerialException as e:
            if attempt == max_attempts - 1:
                raise
            print(f"Попытка {attempt + 1}/{max_attempts}: {e}")
            time.sleep(2)

def send_task(ser, task, *args):
    """Отправляет задачу на Arduino и получает ответ"""
    command = f"{task} {' '.join(map(str, args))}\n"
    try:
        ser.write(command.encode('utf-8'))
        response = ser.readline().decode('utf-8').strip()
        if response:
            print(f"Ответ от Arduino: {response}")
        return response
    except serial.SerialException as e:
        print(f"Ошибка связи: {e}")
        raise

def get_user_input(prompt, validate=None):
    """Безопасный ввод данных с валидацией"""
    while True:
        try:
            value = input(prompt).strip()
            if validate:
                value = validate(value)
            return value
        except ValueError as e:
            print(f"Ошибка: {e}")

def validate_number(value, min_val=0, max_val=255):
    """Проверяет число на вхождение в диапазон"""
    num = int(value)
    if not min_val <= num <= max_val:
        raise ValueError(f"Число должно быть от {min_val} до {max_val}")
    return num

def validate_operation(value):
    """Проверяет допустимость операции"""
    if value not in '+-*/':
        raise ValueError("Допустимые операции: +, -, *, /")
    return value

def validate_expression(expr):
    """Проверяет корректность математического выражения"""
    expr = expr.strip()
    if not expr:
        raise ValueError("Выражение не может быть пустым")
    return expr

def validate_math_expression(expr):
    """Проверяет математическое выражение на допустимые символы"""
    expr = expr.strip()
    if not expr:
        raise ValueError("Выражение не может быть пустым")
    
    allowed_chars = set('0123456789+-*/.() ')
    if not all(c in allowed_chars for c in expr):
        raise ValueError("Выражение содержит недопустимые символы")
    
    return expr

def main():
    try:
        ser = connect_to_arduino()
        
        while True:
            print("\n" + "="*40)
            task = get_user_input(
                "Выберите задачу:\n"
                "1. Умножение двух чисел\n"
                "2. Количество единиц в бинарном представлении\n"
                "3. Деление и остаток\n"
                "4. Возведение в степень\n"
                "5. Математическая операция\n"
                "q. Выход\n"
                "Ваш выбор: ",
                lambda x: x if x in ('1','2','3','4','5','q') else ValueError("Введите 1-5 или q")
            )

            if task == 'q':
                break

            try:
                if task == '1':
                    num1 = get_user_input("Первое число (0-255): ", lambda x: validate_number(x))
                    num2 = get_user_input("Второе число (0-255): ", lambda x: validate_number(x))
                    send_task(ser, task, num1, num2)

                elif task == '2':
                    num = get_user_input("Число (0-255): ", lambda x: validate_number(x))
                    send_task(ser, task, num)

                elif task == '3':
                    num1 = get_user_input("Делимое (0-255): ", lambda x: validate_number(x))
                    num2 = get_user_input("Делитель (0-255): ", lambda x: validate_number(x, 1, 255))
                    send_task(ser, task, num1, num2)

                elif task == '4':
                    num = get_user_input("Число (0-255): ", lambda x: validate_number(x))
                    power = get_user_input("Степень (0-255): ", lambda x: validate_number(x))
                    send_task(ser, task, num, power)

                elif task == '5':
                    expression = get_user_input(
                        "Введите выражение (например: 5+3*2): ",
                        lambda x: validate_math_expression(x)
                    )
                    # Удаляем все пробелы и отправляем на Arduino
                    clean_expr = expression.replace(" ", "")
                    # Отправляем как "5выражение"
                    command = f"5{clean_expr}\n"
                    ser.write(command.encode('utf-8'))
                    # Чтение ответа
                    response = ser.readline().decode('utf-8').strip()
                    if response:
                        print(f"Ответ от Arduino: {response}")


            except ValueError as e:
                print(f"Ошибка ввода: {e}")
            except serial.SerialException:
                print("Попытка переподключения...")
                ser = connect_to_arduino()

    except serial.SerialException as e:
        print(f"Критическая ошибка: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\nПрограмма завершена пользователем")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Соединение закрыто")

if __name__ == "__main__":
    main()