# Use an official GCC image as a parent image
FROM gcc:latest

# Set the working directory in the container
WORKDIR /usr/src/app

# Copy the current directory contents into the container at /usr/src/app
COPY . .

# Install make and cmake
RUN apt-get update && apt-get install -y make

RUN make

# Set default environment variables
ENV SOURCE_PATH=.
ENV THREADS=1

CMD ./bin/mdu $SOURCE_PATH -j $THREADS