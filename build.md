# Build Cerver Docker Images

## Development

Build development image

```
sudo docker build -t ermiry/cmongo:development -f Dockerfile.dev .
```

## Builder

Build builder image

```
sudo docker build -t ermiry/cmongo:builder -f Dockerfile.builder .
```

## Production

Build production image

```
sudo docker build -t ermiry/cmongo:latest -f Dockerfile .
```