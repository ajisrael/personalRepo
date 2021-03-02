const fs = require('fs');

exports.importMessages = () => {
  return fs.readFile('./messages/welcome.txt', 'utf-8', (error, data) => {
    if (error) throw error;
    return data;
  });
};
