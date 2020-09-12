String toString(float value, byte decimals)
{
  String sValue = String(value, decimals);
  sValue.trim();
  return sValue;
}

/*!
   @brief    get the nth part of a comma seperated enum list
    @param    listString  string with comma seperated values
    @param    index       index of string to return starting from 0
    @returns  result String - empty string is index is out of range
*/
String enumGetIndex(String listString, uint8_t index) {
  #ifdef SERIAL_TRACE
    Serial.print(" index:");
    Serial.print(index);
    Serial.print(" list:");
    Serial.print(listString);
  #endif
  if (listString.length()==0) return ""; 
  int startPos = listString.indexOf(',');
  if (index==0) return listString.substring(0,startPos); // 
  if (startPos==-1) return listString; // no commas found: return probing string (?)
  int currentIndex = 1;
  //find start
  while (currentIndex!=index || startPos<0) { // loop until index found or end of string
    Serial.print(".");
    startPos = listString.indexOf(',',startPos+1);
    currentIndex++;
  };
  #ifdef SERIAL_TRACE
    Serial.print(" start:");
    Serial.print(startPos);
  #endif
  if (startPos<0) return ""; // listString has not enough entries
  //find end
  int endPos = listString.indexOf(',',startPos+1);
  if (endPos<0) endPos = listString.length(); // search @ the end of list
  #ifdef SERIAL_TRACE
    Serial.print(" end:");
    Serial.print(endPos);
    Serial.print(" =");
    Serial.println(listString.substring(startPos+1,endPos));
  #endif
  return listString.substring(startPos+1,endPos);
}