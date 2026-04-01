all:
	docker build -t ansg-env -f Dockerfile .
	docker run -it -v $(CURDIR)/workspace:/workspace -p 8080:5900 --name ansg ansg-env

confirm_clean:
	@echo "WARNING: This will delete the docker container and image.\nAre you sure you want to continue? (y/n): " && read ans && [ "$${ans:-N}" = "y" ] || [ "$${ans:-N}" = "Y" ] || (echo "Clean cancelled."; exit 1)

clean: confirm_clean
	@echo "Proceeding with clean..."
	docker container rm ansg
	docker image rm ansg-env
	@echo "Clean complete."
