clear;
close all;
clc;

resp_a = [];
resp_b = [];
rots = [];

for t = -1:0.01:10
    theta = mod(-1.5 * t, 2 * pi);

    A = 0;
    B = 0;

    if theta >= 0 && theta < pi / 2
        A = 5; B = 0;
    elseif theta >= pi / 2 && theta < pi
        A = 5; B = 5;
    elseif theta >= pi && theta < 3 * pi / 2
        A = 0; B = 5;
    else
        A = 0; B = 0;
    end

    rots = [rots, t];
    resp_a = [resp_a; A];
    resp_b = [resp_b; B];

end

size(resp_a)
size(resp_b)

plot(rots, [resp_a, resp_b]);
xlabel('Theta (rad)'); ylabel("Channels Voltage");
ylim([-1, 7])
legend('Channel A', 'Channel B');
