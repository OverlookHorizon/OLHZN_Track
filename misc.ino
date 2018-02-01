char LatitudeStr[16], LongitudeStr[16];
char printLineA[100];

int BuildSentence(char *TxLine, const char *PayloadID){
    int Count, i, j;
    unsigned char c;
    unsigned int CRC, xPolynomial;
	
    SentenceCounter++;
	
    dtostrf(GPS.Latitude, 7, 5, LatitudeStr);
    dtostrf(GPS.Longitude, 7, 5, LongitudeStr);

    snprintf(TxLine,
            SENTENCE_LENGTH-6,
            "$$%s,%d,%s,%s,%d" /*EXTRA_FIELD_FORMAT*/,
            PayloadID,
            SentenceCounter,
            LatitudeStr,
            LongitudeStr,
            (int)GPS.Altitude
            //EXTRA_FIELD_LIST 
            );
            
    /*
            "$$%s,%d,%02d:%02d:%02d,%s,%s,%05.5u,%d,%d,%d",
            PAYLOAD_ID,
            SentenceCounter,
	    GPS.Hours, GPS.Minutes, GPS.Seconds,
            LatitudeStr,
            LongitudeStr,
            GPS.Altitude,
            (int)((GPS.Speed * 13) / 7),
            GPS.Course,
            GPS.Satellites);
    */

    Count = strlen(TxLine);

    CRC = 0xffff;           // Seed
    xPolynomial = 0x1021;
   
     for (i = 2; i < Count; i++)
     {   // For speed, repeat calculation instead of looping for each bit
        CRC ^= (((unsigned int)TxLine[i]) << 8);
        for (j=0; j<8; j++)
        {
            if (CRC & 0x8000)
                CRC = (CRC << 1) ^ 0x1021;
            else
                CRC <<= 1;
        }
     }

    TxLine[Count++] = '*';
    TxLine[Count++] = Hex((CRC >> 12) & 15);
    TxLine[Count++] = Hex((CRC >> 8) & 15);
    TxLine[Count++] = Hex((CRC >> 4) & 15);
    TxLine[Count++] = Hex(CRC & 15);
	TxLine[Count++] = '\n';  
	TxLine[Count++] = '\0';
	
	return strlen(TxLine) + 1;
}
