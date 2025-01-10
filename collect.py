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
        port = self.select_port()
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

if __name__ == "__main__":
    monitor = TMF8821Monitor()
    monitor.run()