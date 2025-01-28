import serial
import serial.tools.list_ports
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
from collections import deque
import time

class TMF8821Monitor:
    def __init__(self):
        self.ser = None
        self.fig = plt.figure(figsize=(4, 4))
        self.ax = self.fig.add_subplot(111, projection='3d')
        
        # 预分配numpy数组以提高性能
        self.x = np.array([0, 0, 0, 1, 1, 1, 2, 2, 2])
        self.y = np.array([0, 1, 2, 0, 1, 2, 0, 1, 2])
        self.z = np.zeros(9, dtype=np.float32)
        self.c = np.zeros(9, dtype=np.float32)
        self.dx = np.full(9, 0.8, dtype=np.float32)
        self.dy = np.full(9, 0.8, dtype=np.float32)
        
        # 数据缓冲
        self.data_buffer = deque(maxlen=3)  # 保存最近3帧数据
        self.judge_buffer = deque(maxlen=10) # 保存最近10帧数据

        # 预计算颜色映射数组
        self.color_map = np.zeros((256, 4))
        for i in range(256):
            self.color_map[i] = plt.cm.viridis(i / 255.0)
        
        # 初始化3D柱状图
        self.bars = self.ax.bar3d(self.x, self.y, np.zeros_like(self.z),
                                 self.dx, self.dy, self.z,
                                 color='b', alpha=0.5)
        
        self._setup_plot()
        
        # 性能优化：关闭自动缩放
        self.ax.autoscale(False)
        
        # 设置更新间隔计时器
        self.last_update = time.time()
        self.update_interval = 0.05  # 50ms

    def _setup_plot(self):
        """设置图表属性的辅助方法"""
        self.ax.set_xlabel('X Position')
        self.ax.set_ylabel('Y Position')
        self.ax.set_zlabel('Distance (mm)')
        self.ax.set_title('TMF8821 3D Distance Map')
        self.ax.view_init(elev=30, azim=45)
        self.ax.set_xlim(-0.5, 2.5)
        self.ax.set_ylim(-0.5, 2.5)
        self.ax.set_zlim(0, 1000)

    def select_port(self):
        # 列出所有可用串口
        ports = list(serial.tools.list_ports.comports())
        print("Available ports:")
        for i, port in enumerate(ports):
            print(f"{i}: {port.device} - {port.description}")
        
        # 让用户选择串口
        selection = int(input("Select port number: "))
        if 0 <= selection < len(ports):
            return ports[selection].device
        return None

    def connect_serial(self):
        port = "COM5"
        if port:
            try:
                self.ser = serial.Serial(port, 115200, timeout=1)
                print(f"Connected to {port}")
                return True
            except serial.SerialException as e:
                print(f"Error connecting to port: {e}")
        return False

    def parse_line(self, line):
        try:
            if line.startswith('#Obj'):
                parts = np.array(line.strip().split(','))
                if len(parts) >= 22:
                    # 使用numpy的矢量化操作
                    confidence_indices = np.arange(7, 25, 2)
                    distance_indices = np.arange(6, 24, 2)
                    
                    confidences = parts[confidence_indices].astype(np.float32)
                    distances = parts[distance_indices].astype(np.float32)
                    
                    # 矢量化的条件操作
                    distances = np.where(confidences > 100, distances, 0)
                    
                    return distances, confidences
            return None, None
        except Exception as e:
            print(f"Parse error: {e}")
            return None, None

    def update_plot(self, frame):
        current_time = time.time()
        if current_time - self.last_update < self.update_interval:
            return ()
        
        if self.ser and self.ser.is_open:
            try:
                # 读取所有可用数据
                while self.ser.in_waiting:
                    line = self.ser.readline().decode('utf-8')
                    result = self.parse_line(line)
                    if result[0] is not None:
                        self.data_buffer.append(result)
                        self.judge_buffer.append(result)
                if self.judge_buffer:
                    determin_direction(self.judge_buffer)
                if self.data_buffer:
                    # 使用最新的数据
                    distances, confidences = self.data_buffer[-1]
                    
                    # 更新数据
                    self.z = distances
                    self.c = confidences
                    
                    # 使用预计算的颜色映射
                    colors = self.color_map[confidences.astype(int)]
                    
                    # 更新图形
                    self.ax.clear()
                    self._setup_plot()
                    self.bars = self.ax.bar3d(self.x, self.y, np.zeros_like(self.z),
                                            self.dx, self.dy, self.z,
                                            color=colors, alpha=0.5)
                    
                    self.last_update = current_time
            
            except Exception as e:
                print(f"Error reading serial: {e}")
        
        return ()

    def run(self):
        if self.connect_serial():
            # 降低动画更新频率，提高性能
            ani = FuncAnimation(self.fig, self.update_plot, 
                              interval=20,  # 降低到20ms
                              blit=False,
                              cache_frame_data=False)  # 禁用帧缓存
            plt.show()
        
        if self.ser:
            self.ser.close()

def get_arrow(dx, dy):
    """根据移动方向返回箭头符号，只返回上下左右四个方向"""
    arrows = {
        (0, -1): "←",    # 左
        (0, 1): "→",     # 右
        (-1, 0): "↑",    # 上
        (1, 0): "↓"      # 下
    }
    
    # 添加方向判断的阈值
    dir_threshold = 0.1
    
    # 判断哪个方向的变化更显著
    if abs(dx) > dir_threshold or abs(dy) > dir_threshold:
        if abs(dx) > abs(dy):
            # 上下移动更明显
            x_dir = 1 if dx > 0 else -1
            y_dir = 0
        else:
            # 左右移动更明显
            x_dir = 0
            y_dir = 1 if dy > 0 else -1
    else:
        return "•"  # 静止状态
    
    return arrows.get((x_dir, y_dir), "•")

class DirectionFilter:
    def __init__(self, buffer_size=5):
        self.direction_buffer = deque(maxlen=buffer_size)
        self.last_direction = "•"
        self.consecutive_count = 0
        self.min_consecutive = 2  # 需要连续出现的次数
    
    def update(self, new_direction):
        if new_direction == self.last_direction:
            self.consecutive_count += 1
        else:
            self.consecutive_count = 1
            
        self.direction_buffer.append(new_direction)
        self.last_direction = new_direction
        
        # 只有当某个方向连续出现足够次数时才返回
        if self.consecutive_count >= self.min_consecutive:
            return new_direction
        return None
    
    def get_dominant_direction(self):
        if not self.direction_buffer:
            return "•"
        # 获取出现最多的方向
        return max(set(self.direction_buffer), 
                  key=self.direction_buffer.count)

def determin_direction(buffer):
    if len(buffer) < 5:
        return
    
    # 使用全局方向过滤器
    global direction_filter
    if 'direction_filter' not in globals():
        direction_filter = DirectionFilter()
    
    recent_frames = list(buffer)[-5:]
    centroids = []
    
    for frame in recent_frames:
        distances = np.array(frame[0]).reshape(3, 3)
        valid_points = distances > 0
        if not valid_points.any():
            continue
            
        x_coords, y_coords = np.where(valid_points)
        g_x = np.mean(x_coords)
        g_y = np.mean(y_coords)
        centroids.append((g_x, g_y))
    
    if len(centroids) < 3:
        print("\r ", end="", flush=True)
        return
        
    x_trend = np.polyfit([i for i in range(len(centroids))],
                        [c[0] for c in centroids], 1)[0]
    y_trend = np.polyfit([i for i in range(len(centroids))],
                        [c[1] for c in centroids], 1)[0]
    
    threshold = 0.1
    
    if abs(x_trend) > threshold or abs(y_trend) > threshold:
        new_arrow = get_arrow(x_trend, y_trend)
        # 使用方向过滤器
        filtered_arrow = direction_filter.update(new_arrow)
        if filtered_arrow:
            print("\r" + filtered_arrow + " ", end="", flush=True)
        else:
            print("\r ", end="", flush=True)
    else:
        direction_filter.update("•")
        print("\r ", end="", flush=True)

class Part:
    Left = 0
    LeftTop = 1
    Top = 2
    RightTop = 3
    Right = 4
    RightDown = 5
    Down = 6
    LeftDown = 7
    Undetermined = 8
class Direction():
    def __init__(self):
        self.start_part = Part.Undetermined
        self.accpetable_part = []
        self.part_history = []
      
    def set_start_part(self, part):
        self.start_part = part
        self.accpetable_part = [0, 1, 2, 3, 4, 5, 6, 7]
        self.accpetable_part.remove(part)
        self.accpetable_part.remove((part + 1) % 8)
        self.accpetable_part.remove((part - 1) % 8)
    

if __name__ == "__main__":
    monitor = TMF8821Monitor()
    monitor.run()