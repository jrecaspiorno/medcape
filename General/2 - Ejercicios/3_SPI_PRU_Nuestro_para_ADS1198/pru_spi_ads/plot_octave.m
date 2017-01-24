function plot_octave()


#0.1. Read and parse the data.
#==========================================================================#
fid = fopen("data_medcape.data");
line = 0;
line_i=1; 
counter_chars = 1;
while (! feof (fid) )
 line = fgetl(fid);
 elems = strsplit(line);
 nums = str2double(elems);
 

 for i=1:columns(line)
  hex_chars(i) = line(i);
  data(counter_chars,:) = hex_chars(i);
  counter_chars += 1;
 endfor
end 
fclose(fid); 

#=======================================================================
number_of_samples = rows(data) / 36; #Each sample = 36 hex characters
time_recorded = 20;
num_channels = 1;
#=======================================================================

Y = zeros(number_of_samples*num_channels, 1);
one_sample = zeros(6, 1); #Array of 6 hex characters

counter_samples = 1;
counter_status_bits = 0;
counter_1_sample = 1;
counter_ints = 1;
counter_chars_taken_channel = 1;
counter_chars_taken = 1;
counter_channels = 1;

counter_channel_per_sample = 0;

for i=1:rows(data)

      if (counter_status_bits == 8)
          hex_char = (data(i, :));
          one_channel(counter_chars_taken_channel) = data(i, :);
          if (num_channels == 1 && counter_channel_per_sample == 1)
            paso = 1;
          else
            if (counter_chars_taken_channel == 6)
              Y(counter_channels) = hex2dec(one_channel);
              counter_chars_taken_channel = 1;
              counter_channels += 1;
              counter_channel_per_sample += 1;
            else 
              counter_chars_taken_channel += 1;
            endif;
          endif;
      else
        counter_status_bits += 1;
      endif;
      
      if (counter_1_sample == 36)
        counter_1_sample = 0;
        counter_status_bits = 1;
        counter_channel_per_sample = 0;
      else
        counter_1_sample += 1;
        counter_chars_taken += 1;
      endif;
     
endfor;


#sample_rate = 500;
sample_rate = 2;

X = sample_rate; #500 Hz: Frecuencia de muestreo (sample rate)  (500 cycles per second)
X = [0:2:2];
#===Debe ser asi en realidad=== 
#X = time_recorded * sample_rate;
#X = [0:1/sample_rate:time_recorded];
#==============================


plot(X, Y);

endfunction