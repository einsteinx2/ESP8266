const RgbColor kTestSequenceLedColors[6] = { RgbColor(128, 0, 0), RgbColor(0, 128, 0), RgbColor(0, 0, 128), 
                                             RgbColor(128, 0, 0), RgbColor(0, 128, 0), RgbColor(0, 0, 128) };

void testSequence()
{
    if (lightsOn)
    {
        static int offset = 0;
        
        for (int i = 0; i < 6; i++)
        {
            int index = i + offset;
            if (index >= 6)
            {
                index = index - 6;
            }
            
            strip.SetPixelColor(i, kTestSequenceLedColors[index]);
        }

        offset++;
        if (offset >= 6)
        {
            offset = 0;
        }
    }
    else
    {
        for (int i = 0; i < 6; i++)
        {
            strip.SetPixelColor(i, RgbColor(0, 0, 0));
        }
    }
    
    strip.Show();
}

