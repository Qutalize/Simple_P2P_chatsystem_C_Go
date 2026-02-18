start:
	docker-compose up -d
stop:
	docker-compose down
login-node1:
	docker exec -it node1 bash
login-node2:
	docker exec -it node2 bash
clean:
	docker-compose down --rmi all --volumes