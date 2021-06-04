import React, { useState } from "react";
import { makeStyles } from "@material-ui/core/styles";
import "./App.css";
import { BLEUART } from "ble-smartconfig";
import Container from "@material-ui/core/Container";
import Button from "@material-ui/core/Button";

type Props = {
  onConnect: (uart: BLEUART) => void
};

const useStyles = makeStyles((theme) => ({
  button: {
    margin: theme.spacing(3, 0, 2),
  },
}));


export function ConnectDevice(props: Props) {
  const classes = useStyles();
  const [connecting, setConnecting] = useState(false);

  if(connecting) {
    return (
      <Container>
        <Button
          disabled
          fullWidth
          variant="contained"
          color="primary"
          className={classes.button}
        >
          Connecting...
        </Button>
      </Container>
    );
  }
  else {
    return (
      <Container>
        <Button
          type="submit"
          fullWidth
          variant="contained"
          color="primary"
          className={classes.button}
          onClick={async () => {
            let uart = new BLEUART("Nordic");
            setConnecting(true);
            await uart.start();
            props.onConnect(uart);
            setConnecting(false);
          }}
        >
          Connect to device
        </Button>
      </Container>
    );
  }
}

