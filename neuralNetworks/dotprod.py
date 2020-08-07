# video link: https://youtu.be/tMrbN67U9d4
import numpy as np

inputs = [1.0, 2.0, 3.0, 2.5]

weights = [[ 0.2,   0.8,  -0.5,  1.0  ],
           [ 0.5,  -0.91,  0.26, -0.5 ],
           [-0.26, -0.27,  0.17,  0.87]]

biases = [2, 3, 0.5]

# Weights needs to come before inputs otherwise shapes don't allign
# Vectors need to be of same length for dot product
output = np.dot(weights, inputs) + biases
print(output)