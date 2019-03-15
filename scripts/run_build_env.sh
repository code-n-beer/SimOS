docker exec -it $(docker run -d --rm --mount type=bind,source="$(pwd)",target=/simos simobuild:latest) bash
