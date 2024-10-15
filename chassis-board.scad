height = 3;
width = 30;
length = 100;

base = [
    [0, 0],
    [0, length],
    [width, length],
    [width, 0]
];

delta_length = 42;
delta_base = [
    [0, 0],
    [0, delta_length],
    [width / 2, delta_length],
    [width / 2, 0]
];

notch = [
    [0, 0], [0, 1], [1, 1], [1, 0]
];


difference() {
    linear_extrude(height = height) {
        polygon(base);
    }
    
    translate([width / 4,0,0])
        linear_extrude(height = height) {
            polygon(delta_base);
        }
        
    // notch for the sensor
    translate([0, length / 4 * 3, 0])
        linear_extrude(height = height) {
            polygon(notch);
        } 
    translate([width - 1, length / 4 * 3, 0])
        linear_extrude(height = height) {
            polygon(notch);
        }
        
    // notch for the board
    translate([0, length / 3, 0])
        linear_extrude(height = height) {
            polygon(notch);
        } 
    translate([width - 1, length / 3, 0])
        linear_extrude(height = height) {
            polygon(notch);
        }
}