Point(1)  = {0.5, 0.0, 0.0};
Point(2)  = {0.5, 0.5, 0.0};
Point(3)  = {0.375, 0.5, 0.0};
Point(4)  = {0.375, 1.0, 0.0};
Point(5)  = {0.125, 1.0, 0.0};
Point(6)  = {0.125, 0.5, 0.0};
Point(7)  = {0.0, 0.5, 0.0};
Point(8)  = {0.0, 0.0, 0.0};

Point(9)  = {0.5, 0.0, 0.2};
Point(10) = {0.5, 0.5, 0.2};
Point(11) = {0.375, 0.5, 0.2};
Point(12) = {0.375, 1.0, 0.2};
Point(13) = {0.125, 1.0, 0.2};
Point(14) = {0.125, 0.5, 0.2};
Point(15) = {0.0, 0.5, 0.2};
Point(16) = {0.0, 0.0, 0.2};

Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 5};
Line(5) = {5, 6};
Line(6) = {6, 7};
Line(7) = {7, 8};
Line(8) = {8, 1};
Line(9) = {3, 6};

Line(10) = {9, 10};
Line(11) = {10, 11};
Line(12) = {11, 12};
Line(13) = {12, 13};
Line(14) = {13, 14};
Line(15) = {14, 15};
Line(16) = {15, 16};
Line(17) = {16, 9};
Line(18) = {11, 14};

Line(19) = {1, 9};
Line(20) = {2, 10};
Line(21) = {3, 11};
Line(22) = {4, 12};
Line(23) = {5, 13};
Line(24) = {6, 14};
Line(25) = {7, 15};
Line(26) = {8, 16};

Line Loop(27) = {1, 20, -10, -19};
Plane Surface(28) = {27};
Line Loop(29) = {2, 21, -11, -20};
Plane Surface(30) = {29};
Line Loop(31) = {3, 22, -12, -21};
Plane Surface(32) = {31};
Line Loop(33) = {4, 23, -13, -22};
Plane Surface(34) = {33};
Line Loop(35) = {5, 24, -14, -23};
Plane Surface(36) = {35};
Line Loop(37) = {6, 25, -15, -24};
Plane Surface(38) = {37};
Line Loop(39) = {7, 26, -16, -25};
Plane Surface(40) = {39};
Line Loop(41) = {8, 19, -17, -26};
Plane Surface(42) = {41};
Line Loop(43) = {1, 2, 9, 6, 7, 8};
Plane Surface(44) = {43};
Line Loop(45) = {3, 4, 5, -9};
Plane Surface(46) = {45};
Line Loop(47) = {10, 11, 18, 15, 16, 17};
Plane Surface(48) = {47};
Line Loop(49) = {12, 13, 14, -18};
Plane Surface(50) = {49};

Surface Loop(51) = {32, 46, 34, 36, 38, 44, 28, 30, 48, 50, 40, 42};
Volume(52) = {51};

Physical Surface(53) = {28};
Physical Surface(54) = {30};
Physical Surface(55) = {32};
Physical Surface(56) = {34};
Physical Surface(57) = {36};
Physical Surface(58) = {38};
Physical Surface(59) = {40};
Physical Surface(60) = {42};
Physical Surface(61) = {44, 46};
Physical Surface(62) = {48, 50};

Physical Volume(63) = {52};
