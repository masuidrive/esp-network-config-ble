import { BLESmartConfigWizard } from '@masuidrive/esp-network-config-ble';
import React from 'react';

const StartupComponent = props => {
  console.log(props);


  return (<div>state: {props.state}</div>);
};

export class App extends React.Component {
  constructor(props) {
    super(props);
    this.wizard = new BLESmartConfigWizard("A1-", async (deviceId) => {
      const r = await fetch('./mqtt-config.json');
      return await r.json();
    });
    this.state = {
      page: "startup",
      params: { },
    }
    this.wizard.pageStartup.on((params) => this.setState({params}));
  }

  componentDidMount() {
    this.wizard.start();
  }

  render() {
    let Page = StartupComponent;

    return (
      <div className="App">
        <Page {...this.state.params}/>
      </div>
    );
  }
}
