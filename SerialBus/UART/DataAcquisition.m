clear
close all
clc
s = serial('/dev/ttyACM0');
s.BaudRate = 115200;
s.InputBufferSize = 6000;
fopen(s);
while(1)
    try
        while(s.BytesAvailable == 0)
        end
        fscanf(s,'%s')
        g = input('Inserisci numero campioni --> ');
        nCifre = ceil(log10(g));
        fwrite(s,num2str(nCifre),'uchar');

        fwrite(s,num2str(g),'uchar');
 
        while(s.BytesAvailable == 0)
        end
        data1 = fscanf(s,'%s');
        data = str2num(data1)*3000/4096;
        plot(data)
        xlabel('Samples','FontName','Times','FontSize',16)
        ylabel('Voltage [mV]','FontName','Times','FontSize',16)
        set(gca,'FontName','Times','FontSize',14)
        pause(0.01)
        clc
    catch
        fclose(s)
        delete(s)
        clear s
    end
end
fclose(s)
delete(s)
clear s
