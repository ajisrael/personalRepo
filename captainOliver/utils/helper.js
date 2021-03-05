const fs = require('fs');

exports.importMessages = () => {
  let messages = {};

  text = fs.readFileSync('./messages/welcome.txt', 'utf-8', (error, data) => {
    if (error) throw error;
    return data;
  });

  content = text.split('\n');
  content.forEach((line) => {
    let messageName = '';
    if (line.startsWith('##')) {
      messageName = line.split('messageName: ')[1];
      messages[messageName] = [];
    } else {
      messages[messageName] += line + '\n';
    }
  });
  console.log(messages.welcome);
};
