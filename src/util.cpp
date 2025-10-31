#include "util.h"

template<typename T>
ring_buf<T>::page_t::page_t(){
  size = getpagesize();
  addr = new std::byte[size];
  in_use = false;
}
   
template<typename T>
ring_buf<T>::page_t::~page_t(){
  delete[] addr;
}
template<typename T>
size_t ring_buf<T>::mmove(std::span<std::byte> dest){
  if(dest.size_bytes()+start.offset < chunks[start.chunk].size){
    memcpy(dest.data(),&chunks[start.chunk].addr[start.offset],dest.size_bytes());
  } else {
    std::vector<std::byte> valarray;
    valarray.resize(dest.size_bytes());
    for(int i = 0; i < valarray.size(); i++){
      address_t loc = addrAt(i);
      valarray[i] = chunks[loc.chunk].addr[loc.offset];
    }
    memcpy(dest.data(),valarray.data(),dest.size_bytes());
  }
  
  start = addrAt(dest.size_bytes());
  len -= dest.size_bytes();
  return dest.size_bytes();
}


template<typename T>
void ring_buf<T>::resize(size_t count){
  chunks.resize((sizeof(T)*count)/(getpagesize()));
  len = count;
}

template<typename T>
size_t ring_buf<T>::size(){
  return len;
  /*if(end.chunk == start.chunk)
    if(start.offset <= end.offset){
      return end.offset - start.offset;
    } else {
      return 
    }*/
}
template<typename T>
T *ring_buf<T>::data(){
  return reinterpret_cast<T*>(&chunks[start.chunk].addr[start.offset]);
}

template<typename T>
ring_buf<T>::address_t ring_buf<T>::addrAt(size_t index){
  size_t chunk_num = start.chunk;
  size_t off_bytes = start.offset;
  off_bytes += index;
  chunk_num += off_bytes / getpagesize();
  off_bytes %= getpagesize();
  return {chunk_num,off_bytes};

}

template class ring_buf<std::byte>;
template class ring_buf<rgb888>;

template<typename T>
void ring_buf<T>::append(std::byte val){
  end.offset += 1;
  if(end.offset >= chunks[end.chunk].size){
    ++end.chunk;
    end.offset = 0;
    if(end.chunk >= chunks.size()){
      end.chunk = 0;
    }
  }
  chunks[end.chunk].addr[end.offset] = val;  
}

constexpr std::string colorTypeString(uint8_t val){ 
  switch (val){
    case 0:
      return "0: Greyscale";
    case 1:
      return "2: Truecolor";//there is no color type 1
    case 3:
      return "3: Indexed Color (palette)";
    case 4:
      return "4: Greyscale with alpha";
    case 6:
      return "6: Truecolor with alpha";//same here, no color type 5
    default:
      return std::format("\nERR: Invalid color type {:d}", val);
  }
}


constexpr std::string zlib_return_string(int val){
  std::string ret;
  if(val< 0){ ret = "ERR: zlib returned error"; }
  if(val==Z_OK){ret		="okay (Z_OK)"; } else
  if(val==Z_STREAM_END){ret	="stream end (Z_STREAM_END)";} else 
  if(val==Z_NEED_DICT){ret 	="needs dictionary (Z_NEED_DICT)";} else
  if(val==Z_ERRNO){ret	 	="(Z_ERRNO), check errno ";} else
  if(val==Z_STREAM_ERROR){ret	="(Z_STREAM), stream error";} else
  if(val==Z_DATA_ERROR){ret	="(Z_DATA), data error";} else
  if(val==Z_MEM_ERROR){ret	="(Z_MEM), MEM error";} else
  if(val==Z_BUF_ERROR){ret	="(Z_BUF), BUF error";} else
  if(val==Z_VERSION_ERROR){ret	="(Z_VERSION), VERSION error";} 
  return ret;
}


