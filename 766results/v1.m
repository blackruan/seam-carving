clear all
close all
clc

%v= VideoReader('seam-ind.mov');
%v= VideoReader('seam-our.mov');
v= VideoReader('valkyrie_wolf.mov');
%video= zeros(240,320,3,82);
k= 1;
while hasFrame(v)
    video(:,:,:,k) = readFrame(v);
    k= k+1;
end

%return

%%kk= [1,10,20,40,80];
kk= [k-1];

for i=1:length(kk)
    figure;
    imshow(uint8(video(:,:,:,kk(i))))
end