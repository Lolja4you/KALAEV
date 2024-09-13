import math

class GlobalState:
    def __init__(self):
        self.chamber = Chamber()
        self.pumps = Pumps()

        self.pressure_measurement_units = {
            "Bar": 0,
            "Pascal": 1,
            "millimet of mercury": 2,
        }
        self.pumping_curve_coefficient = {
            "F1": [1000, 250, 1],
            "F1_5": [250, 100, 1.5],
            "F1_75": [100, 50, 1.75],
            "F2": [50, 20, 2],
            "F2_5": [20, 5, 2.5],
            "F3": [5, 1, 3],
        }

        self.pumping_time = []

    def get_current_pumping_curve_coefficient(self, current_pressure):
        closest_key = None
        closest_value = float('inf')

        for key, value in self.pumping_curve_coefficient.items():
            if value[0] >= current_pressure and value[0] < closest_value:
                closest_value = value[0]
                closest_key = key
        
        if closest_key:
            return (closest_key, self.pumping_curve_coefficient[closest_key])
        
        return None

    def calculate_pumping_times(self):
        current_pressure = self.chamber.start_pressure
        while current_pressure > self.chamber.end_pressure:
            pump = self.pumps.get_pump(current_pressure)
            print('lol')
            if pump is None:
                break
            print('lo')
            current_pumping_curve = self.get_current_pumping_curve_coefficient(current_pressure)
            if current_pumping_curve is None:
                current_pumping_curve = ('F1', self.pumping_curve_coefficient['F1'])
            print('l')
            curve_key, curve_values = current_pumping_curve
            time = (self.chamber.volume / pump.pump_performance) * \
                   math.log(2 * (curve_values[0] / curve_values[1]))
            
            self.pumping_time.append((curve_key, time))
            current_pressure = curve_values[1]


    def get_pumping_time(self):
        self.calculate_pumping_times()
        return self.pumping_time

class Chamber:
    def __init__(self, start_pressure=10000, end_pressure=0.0000133322, volume=0.12):
        self.start_pressure = start_pressure
        self.end_pressure = end_pressure
        self.volume = volume
        self.current_pressure = start_pressure

class Pumps:
    def __init__(self):
        self.pumps = []

    def get_pump(self, current_pressure):
        for pump in self.pumps:
            if pump.maximum_residual_pressure < current_pressure:
                return pump
        print("Нет доступного насоса для текущего давления.")
        return None

    def add_pump(self, pump_name, pump_type, pump_performance, maximum_residual_pressure):
        pump_id = len(self.pumps) + 1
        new_pump = Pump(pump_id, pump_name, pump_type, pump_performance, maximum_residual_pressure)
        self.pumps.append(new_pump)
        print(f"Насос {pump_name} добавлен.")

    def list_pump(self):
        if not self.pumps:
            print("Список насосов пуст.")
            return

        print("Список насосов:")
        for pump in self.pumps:
            print(pump)

    def del_pump(self, id):
        try:
            self.pumps.pop(id)
        except IndexError as e:
            print(e)

class Pump:
    def __init__(self, pump_id, pump_name, pump_type, pump_performance, maximum_residual_pressure):
        self.pump_id = pump_id
        self.pump_name = pump_name
        self.pump_type = pump_type
        self.pump_performance = pump_performance
        self.maximum_residual_pressure = maximum_residual_pressure

    def __str__(self):
        return (f"ID: {self.pump_id}, Название: {self.pump_name}, Тип: {self.pump_type}, "
                f"Производительность: {self.pump_performance}, Максимальное остаточное давление: {self.maximum_residual_pressure}")

# Пример использования
global_state = GlobalState()

# Добавляем предопределенные насосы
global_state.pumps.add_pump("нвр-16д", 0, 63.36, 0.067)
global_state.pumps.add_pump("НВДМ-400", 1, 8460, 0.00000066)

pumping_times = global_state.get_pumping_time()
for curve_key, time in pumping_times:
    print(f"Коэффициент: {curve_key}, Время: {time} сек")
print(global_state.pumping_time)