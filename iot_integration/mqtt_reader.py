from paho.mqtt import client as mqtt_client
from elasticsearch import Elasticsearch

BROKER_IP = "192.168.1.18" # MQTT Broker on my local network
BROKER_PORT = 2222 # MQTT Broker Port
MQTT_TOPIC = "home/firesystem"
CLIENT_ID= "python-esp-elastic"

INDEX_NAME = "fire_system"
ELASTIC_IP = "34.101.197.32" # GCP server that hosts Elasticsearch
ELASTIC_PORT = "9200"

es = Elasticsearch(f"{ELASTIC_IP}:{ELASTIC_PORT}")

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)
    # Set Connecting Client ID
    client = mqtt_client.Client(CLIENT_ID)
    client.on_connect = on_connect
    client.connect(BROKER_IP, BROKER_PORT)
    return client

def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        temp, smoke = msg.payload.decode().split("/")
        print(f"temp={float(temp)},smoke={int(float(smoke))}")

        es.index(
            index=INDEX_NAME,
            document= {
                "smoke": int(float(smoke)),
                "temp": float(temp)
            }
        )
    client.subscribe(MQTT_TOPIC)
    client.on_message = on_message

if __name__ == "__main__":
    client = connect_mqtt()
    
    print("Sending message...")
    client.publish(MQTT_TOPIC, "CLOUD_CONNECTED")

    print(f"Waiting for message from '{MQTT_TOPIC}' ...")
    subscribe(client)
    client.loop_forever()
