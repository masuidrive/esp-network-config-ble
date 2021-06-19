import React, { useState, useEffect } from "react";
import { makeStyles } from "@material-ui/core/styles";
import "./App.css";
import { IoTConfig } from "ble-smartconfig";
import Container from "@material-ui/core/Container";
import TextField from '@material-ui/core/TextField';

type Props = {
  config: IoTConfig,
  onChange: (iotConfig: IoTConfig) => void
};

export function AWSConfigForm(props: Props) {
  return (
    <Container>
      <form noValidate>
        <TextField
          variant="outlined"
          margin="normal"
          required
          fullWidth
          id="mqtt_host"
          label="MQTT Host"
          name="mqtt_host"
          value={props.config.mqtt_host}
          onChange={(e) =>
            props.onChange({...props.config, mqtt_host: e.target.value})
          }
        />

        <TextField
          variant="outlined"
          margin="normal"
          required
          fullWidth
          id="mqtt_port"
          label="MQTT Port"
          name="mqtt_port"
          value={props.config.mqtt_port}
          onChange={(e) =>
            props.onChange({...props.config, mqtt_port: e.target.value})
          }
        />

        <TextField
          variant="outlined"
          margin="normal"
          required
          fullWidth
          id="client_id"
          label="AWS IoT ClientID"
          name="client_id"
          value={props.config.client_id}
          onChange={(e) =>
            props.onChange({...props.config, client_id: e.target.value})
          }
        />

        <TextField
          variant="outlined"
          margin="normal"
          required
          fullWidth
          id="mqtt_topic"
          label="MQTT Topic"
          name="mqtt_topic"
          value={props.config.mqtt_topic}
          onChange={(e) =>
            props.onChange({...props.config, mqtt_topic: e.target.value})
          }
        />

        <TextField
          variant="outlined"
          margin="normal"
          required
          fullWidth
          multiline
          id="root_ca"
          label="AWS IoT AWS IoT Root CA"
          name="root_ca"
          value={props.config.root_ca}
          onChange={(e) =>
            props.onChange({...props.config, root_ca: e.target.value})
          }
        />

        <TextField
          variant="outlined"
          margin="normal"
          required
          fullWidth
          multiline
          id="iot_cert"
          label="AWS IoT Device Certficate"
          name="root_ca"
          value={props.config.cert}
          onChange={(e) =>
            props.onChange({...props.config, cert: e.target.value})
          }
        />

        <TextField
          variant="outlined"
          margin="normal"
          required
          fullWidth
          multiline
          id="priv"
          label="AWS IoT Private key"
          name="priv"
          value={props.config.priv}
          onChange={(e) =>
            props.onChange({...props.config, priv: e.target.value})
          }
        />

      </form>
    </Container>
  );

}

