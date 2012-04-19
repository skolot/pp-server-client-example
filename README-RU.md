Ping-Pong сервер-клиент

два варианта реализации, классический с Unix socket + на собственных syscall 

* ./kernel - сисколы для ядра, необходимо скопировать в дириктории с исходниками ядра linux 
patches - патчи добавляющие мои syscall's 
pp-mycom-server и pp-mycom-client - пинг-понг через syscall'ы  
pp-socket-server и pp-socket-client - пинг-понг через unix socket 


