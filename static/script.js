let count = 0;

document.getElementById('clickButton').addEventListener('click', function() {
    count++;
    document.getElementById('counter').textContent = count;
});
