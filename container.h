#include <iostream>
#include <cstring>

#ifndef __Z_CONTAINER
#define __Z_CONTAINER

constexpr size_t META_WIDTH = sizeof(size_t);
constexpr size_t BODY_WIDTH = sizeof(size_t);

// TYPE1
// client to server
struct z_task 
{

};

// TYPE2
/**
 * META_WIDTH/BODY_WIDTH://META_DATA/BODY_DATA
 * 
 * TOTAL_SIZE = META_WIDTH + BODY_WIDTH + 3 + META_DATA_SIZE + BODY_DATA_SIZE
 */
struct z_response 
{
private:
     // ACK NUMBER, SEQ NUMBER
     size_t _response_task_id, _response_id;
     // std::string message;
     char* message {nullptr};

     z_response() noexcept {}

public:
     char* serialize(size_t*) noexcept;
     static z_response deserialize(char* s) noexcept;

     void display() noexcept 
     {
          std::cout << "response_task_id: " << _response_task_id << std::endl;
          std::cout << "response_id: " << _response_id << std::endl;
          if (message != nullptr) 
          {
               std::cout << "message: " << message << std::endl;
               std::cout << "message size: " << strlen(message) << std::endl;
          }
          
     }

     void set_message(const char* msg) noexcept 
     {
          if (msg == nullptr) return;

          size_t len = strlen(msg);
          message = (char*)malloc(len + 1);

          message[len] = '\0';
          memcpy(message, msg, len);
     }

     char* get_message() noexcept
     {
          return message;
     }

     z_response(size_t task_id, size_t id, const char* msg = nullptr) noexcept
     {
          this->_response_task_id = task_id;
          this->_response_id = id;
          this->set_message(msg);
     }

     ~z_response() noexcept
     {
          if (this->message != nullptr)
          {
               free(this->message);
          }
     }

     z_response(const z_response& resp) noexcept
     {
          this->operator=(std::forward<decltype(resp)>(resp));
     }

     z_response(z_response&& resp) noexcept
     {
          this->operator=(std::forward<decltype(resp)>(resp));
     }

     z_response& operator=(const z_response& resp) noexcept
     {
          this->_response_task_id = resp._response_task_id;
          this->_response_id = resp._response_id;
          if (resp.message != nullptr)
          {
               this->set_message(resp.message);
          }
          return *this;
     }

     z_response& operator=(z_response&& resp) noexcept
     {
          this->_response_task_id = std::move(resp._response_task_id);
          this->_response_id = std::move(resp._response_id);
          this->message = std::move(resp.message);
          return *this;
     }
};

constexpr size_t Z_RESPONSE_META_SIZE = sizeof(z_response);

char* z_response::serialize(size_t* len) noexcept 
{
     size_t CURRENT_SIZE = META_WIDTH + BODY_WIDTH + (3 * sizeof(char)) + Z_RESPONSE_META_SIZE + strlen(message);
     char* mem = (char*)malloc(CURRENT_SIZE + 1);
     memset(mem, 0, CURRENT_SIZE);
     mem[CURRENT_SIZE] = '\0';
     *len = CURRENT_SIZE + 1;

     printf("serialized ideal length: %ld\n", CURRENT_SIZE);

     // get meta_value and body_value:
     //   meta_value: Z_PESPONSE_META_SIZE
     //   BODY_WIDTH: other content size
     size_t* meta_val = (size_t*)mem;
     size_t* body_val = (size_t*)(mem + META_WIDTH);
     memcpy(meta_val, &Z_RESPONSE_META_SIZE, sizeof(size_t));
     // *meta_val = Z_RESPONSE_META_SIZE;
     if (message != nullptr)
     {
          size_t len = strlen(message);
          memcpy(body_val, &len, sizeof(size_t));
     }
     else *body_val = 0;

     // "://"
     char* ptr = mem + META_WIDTH + BODY_WIDTH;
     memcpy(ptr, "://", 3 * sizeof(char));

     // serialize current z_response to a string
     char* current_ptr = (char*)this;
     char* mem_ptr = ptr + (3 * sizeof(char));
     memcpy(mem_ptr, current_ptr, Z_RESPONSE_META_SIZE);

     // serialize other content in the tail of before
     if (message != nullptr)
     {
          memcpy(mem_ptr + Z_RESPONSE_META_SIZE, message, *body_val);
     }

     return mem;
}

z_response z_response::deserialize(char* s) noexcept 
{
     // get meta_val and body_val
     size_t* meta_val = (size_t*)s;
     size_t* body_val = (size_t*)(s + META_WIDTH);

     // begin of z_response
     char* ptr = s + META_WIDTH + BODY_WIDTH + (3 * sizeof(char));

     // deserialize z_response
     z_response ret;
     memcpy(&ret, ptr, Z_RESPONSE_META_SIZE);
     ret.message = nullptr;

     // begin of other content
     ptr += Z_RESPONSE_META_SIZE;

     // copy other content into buffer
     if (*body_val != 0) 
     {
          char* buffer = (char*)malloc((*body_val) + 1);
          buffer[(*body_val)] = '\0';
          memcpy(buffer, ptr, *body_val);
          // make buffer move to message
          ret.message = buffer;
     }

     // return deserialized z_response
     return ret;
}

#endif