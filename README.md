# stm32_bootloader
STM32 bootloader driver via MQTT , JSON , SMING 

topics are structured like this
<prefix>/stm32/in/<topic> : to send something to driver, who is sucbsribed to <prefix>/stm32/in/#
Topics
- request : JSON structure for incoming request { "cmd" : string, "id":number ,"data":string base64 encoded }
the data is a strean of byte codes steering the driver
..*X_RESET : toggle reset pin and send SYNC byte to stm32 , more details see STM32 bootloader protocol
..*X_WAIT_ACK : wait 10 msec on an ACK , return error if failed
..*X_SEND <count> : send count+1 bytes
..*X_BOOT0 <high/low> : set boot pin 
..*X_RECV_VAR : receive variable number of bytes
..*X_RECV <count> : receive count+1 bytes

<prefix>/stm32/<topic> : topics to which the device/driver is publishing data
Topics :
log : log output of device
reply : JSON object containng result data and error code


