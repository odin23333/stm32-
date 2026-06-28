import sensor, image, time, lcd

# 初始化摄像头和LCD
lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)  # 320x240
sensor.skip_frames(time=100)
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)
clock = time.clock()

# 定义红、绿、黄三种颜色的LAB阈值
# 格式: (L_min, L_max, A_min, A_max, B_min, B_max)
# 注意：这些阈值可能需要根据实际环境调整

# 红色阈值 (LAB色彩空间)
red_threshold = (30, 70, 40, 80, 20, 70)

# 绿色阈值 (LAB色彩空间)
green_threshold = (30, 70, -70, -20, 10, 60)

# 黄色阈值 (LAB色彩空间)
yellow_threshold = (50, 90, -20, 20, 50, 90)

# 颜色阈值列表和对应的颜色名称
color_thresholds = [
    (red_threshold, "RED", (255, 0, 0)),       # 红色
    (green_threshold, "GREEN", (0, 255, 0)),    # 绿色
    (yellow_threshold, "YELLOW", (255, 255, 0)) # 黄色
]

print("Color Ball Detection Started!")
print("Detecting: RED, GREEN, YELLOW balls")

while True:
    clock.tick()
    img = sensor.snapshot()

    # 统计每种颜色的球数量
    red_count = 0
    green_count = 0
    yellow_count = 0

    # 检测每种颜色的球
    for threshold, color_name, draw_color in color_thresholds:
        blobs = img.find_blobs([threshold],
                               pixels_threshold=200,    # 最小像素数
                               area_threshold=200,      # 最小面积
                               merge=True,              # 合并相邻色块
                               margin=10)               # 合并边距

        for blob in blobs:
            # 绘制矩形框和十字标记
            img.draw_rectangle(blob.rect(), color=draw_color)
            img.draw_cross(blob.cx(), blob.cy(), color=draw_color)

            # 根据颜色名称计数
            if color_name == "RED":
                red_count += 1
            elif color_name == "GREEN":
                green_count += 1
            elif color_name == "YELLOW":
                yellow_count += 1

            # 在色块旁边显示颜色名称
            img.draw_string(blob.x(), blob.y() - 15, color_name, color=draw_color, scale=1.5)

    # 在LCD上显示统计信息
    fps = clock.fps()

    # 第一行：显示FPS
    img.draw_string(0, 0, "FPS:%2.1f" % fps, color=(255, 255, 255), scale=2.0)

    # 第二行：红色数量
    img.draw_string(0, 30, "Red:%d" % red_count, color=(255, 0, 0), scale=2.0)

    # 第三行：绿色数量
    img.draw_string(0, 60, "Green:%d" % green_count, color=(0, 255, 0), scale=2.0)

    # 第四行：黄色数量
    img.draw_string(0, 90, "Yellow:%d" % yellow_count, color=(255, 255, 0), scale=2.0)

    # 总数
    total = red_count + green_count + yellow_count
    img.draw_string(0, 120, "Total:%d" % total, color=(255, 255, 255), scale=2.0)

    lcd.display(img)

    # 打印到串口
    print("Red:%d Green:%d Yellow:%d Total:%d" % (red_count, green_count, yellow_count, total))
