#ifndef UTILS_H__
#define UTILS_H__

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


/**** sort arrays https://github.com/emilv/ArduinoSort/blob/master/ArduinoSort.h ****/

// Sort an array
template<typename AnyType> void sortArray(AnyType array[], size_t sizeOfArray);

// Sort in reverse
template<typename AnyType> void sortArrayReverse(AnyType array[], size_t sizeOfArray);

// Sort an array with custom comparison function
template<typename AnyType> void sortArray(AnyType array[], size_t sizeOfArray, bool (*largerThan)(AnyType, AnyType));

// Sort in reverse with custom comparison function
template<typename AnyType> void sortArrayReverse(AnyType array[], size_t sizeOfArray, bool (*largerThan)(AnyType, AnyType));

/**** Implementation below. Do not use below functions ****/

namespace ArduinoSort {
	template<typename AnyType> bool builtinLargerThan(AnyType first, AnyType second) {
		return first > second;
	}

	template<> bool builtinLargerThan(char* first, char* second) {
		return strcmp(first, second) > 0;
	}

	template<typename AnyType> void insertionSort(AnyType array[], size_t sizeOfArray, bool reverse, bool (*largerThan)(AnyType, AnyType)) {
		for (size_t i = 1; i < sizeOfArray; i++) {
			for (size_t j = i; j > 0 && (largerThan(array[j-1], array[j]) != reverse); j--) {
				AnyType tmp = array[j-1];
				array[j-1] = array[j];
				array[j] = tmp;
			}
		}
	}
}

template<typename AnyType> void sortArray(AnyType array[], size_t sizeOfArray) {
	ArduinoSort::insertionSort(array, sizeOfArray, false, ArduinoSort::builtinLargerThan);
}

template<typename AnyType> void sortArrayReverse(AnyType array[], size_t sizeOfArray) {
	ArduinoSort::insertionSort(array, sizeOfArray, true, ArduinoSort::builtinLargerThan);
}

template<typename AnyType> void sortArray(AnyType array[], size_t sizeOfArray, bool (*largerThan)(AnyType, AnyType)) {
	ArduinoSort::insertionSort(array, sizeOfArray, false, largerThan);
}

template<typename AnyType> void sortArrayReverse(AnyType array[], size_t sizeOfArray, bool (*largerThan)(AnyType, AnyType)) {
	ArduinoSort::insertionSort(array, sizeOfArray, true, largerThan);
}

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _b : _a; })

#endif