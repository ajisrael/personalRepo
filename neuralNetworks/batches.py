# video link: https://youtu.be/TEWy9vZcxW4
import numpy as np

# A batch is a set of input vectors to help generalize 
# data for the neural network
inputs =  [[ 1.0 ,  2.0 ,  3.0 ,  2.5],
           [ 2.0 ,  5.0 , -1.0 ,  2.0],
           [-1.5 ,  2.7 ,  3.3 , -0.8]]

weights = [[ 0.2 ,  0.8 , -0.5 ,  1.0 ],
           [ 0.5 , -0.91,  0.26, -0.5 ],
           [-0.26, -0.27,  0.17,  0.87]]

biases = [2, 3, 0.5]

# Inputs and weights have the same shape (3, 4). To solve this
# we need to transpose the weights matrix to make it's shape (4, 3).
output = np.dot(inputs, np.array(weights).T) + biases
print(output)