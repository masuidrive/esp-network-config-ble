import React, { useState } from "react";
import { makeStyles } from "@material-ui/core/styles";
import "./App.css";
import { BLESmartConfig, BLEUART, State, States } from "ble-smartconfig";
import Container from "@material-ui/core/Container";
import Button from "@material-ui/core/Button";
import CircularProgress from '@material-ui/core/CircularProgress';

type Props = {
  onConnect: (smartConfig: BLESmartConfig) => void
};

const useStyles = makeStyles((theme) => ({
  button: {
    margin: theme.spacing(3, 0, 2),
  },
  buttonProgess: {
    "margin-right": ".5em"
  }
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
          <CircularProgress size={18} className={classes.buttonProgess}/> Connecting...
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
            let smartConfig = new BLESmartConfig(uart,(
              prevState: States,
              currentState: States
            ) => {
              console.log("state:", prevState, currentState);
            });
            setConnecting(true);
            await smartConfig.connect();
            props.onConnect(smartConfig);
            setConnecting(false);
          }}
        >
          Connect to device
        </Button>
      </Container>
    );
  }
}

