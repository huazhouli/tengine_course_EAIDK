import time
from tengine import tg
import numpy as np
import cv2
# import os
# os.environ['LD_LIBRARY_PATH'] = '/home/shawn/tengine/install/lib/'

model = 'models/mobilenet.tmfile'
image = 'images/cat.jpg'
repeat_count = 1
scale = 0.017
mean = [104.007, 116.669, 122.679]


def parse_data():
    img0 = cv2.imread(image, -1)
    img = cv2.resize(img0, (224, 224))
    input_data, input_0, input_1, input_2 = img, [], [], []
    input_data = (input_data - mean) * scale
    for d in input_data.astype(np.float32).reshape((-1, 3)):
        input_0.append(d[0])
        input_1.append(d[1])
        input_2.append(d[2])
    input_data = input_0 + input_1 + input_2
    input_data = np.array(input_data)
    return input_data, img0


def classify():
    image_data, img = parse_data()
    graph = tg.Graph(None, 'tengine', model)

    graph.preRun()

    max_time = 0
    min_time = None
    whole_time1 = time.time()
    for i in range(repeat_count):
        time_1 = time.time()
        graph.run(block=1, input_data=image_data)
        time_2 = time.time()
        spend_time = time_2 - time_1
        if spend_time > max_time:
            max_time = spend_time
        if i == 0:
            min_time = spend_time
        if spend_time < min_time:
            min_time = spend_time
    whole_time2 = time.time()
    avg_time = (whole_time2 - whole_time1) / repeat_count
    print('Repeat {} times, avg time per run is {} ms'.format(repeat_count, round(avg_time, 2)))
    print('max time is {} ms, min time is {} ms'.format(round(max_time, 2), round(min_time, 2)))

    output_tensor = graph.getOutputTensor(0, 0)
    data = output_tensor.getbuffer(float)

    t_list = data[0: 1000]

    with open('models/synset_words.txt', 'r') as f:
        labels = f.readlines()

    pairs = []
    for idx, k in enumerate(t_list):
        pair = [idx, k]
        pairs.append(pair)

    pairs.sort(key=lambda x: x[1], reverse=True)
    print('-'*40)
    for i in range(5):
        idx = pairs[i][0]
        print(('%.4f' % t_list[idx]) + '-' + labels[idx].replace('\n', ''))
    print('-' * 40)
    cv2.putText(img, labels[pairs[0][0]].strip(), (30, 30), cv2.FONT_HERSHEY_PLAIN, 2.0, (0, 0, 255), 2)
    cv2.imshow('image', img)
    k = cv2.waitKey(0)
    if k == 23:
        cv2.destroyAllWindows()
    
    graph.postRun()


if __name__ == '__main__':
	classify()
