//require from node modules
const mqtt = require('mqtt')
const mysql = require('mysql');

//mqtt configuration
const host = 'broker.emqx.io'
const port = '1883'
const clientId = `mqttx_c6a5d529` //mqttx_4e57a5ec
const user = 'Yusri'
const password = 'Esp@32'

//mysql configuration
const connection = mysql.createConnection({
  host : 'localhost',
  user : 'root',
  password : '',
  database : 'mqtt_log'
});

const connectUrl = `mqtt://${host}:${port}`
const client = mqtt.connect(connectUrl, {
  clientId,
  clean: true,
//   connectTimeout: 4000,
//   username: 'emqx',
//   password: 'public',
//   reconnectPeriod: 1000,
  keepAlive: 60000
})

const topic = 'ESP32BLE/ATTENDANCESYSTEM'
client.on('connect', () => {
  console.log('Connected')
  client.subscribe([topic], () => {
    console.log(`Subscribe to topic '${topic}'`)
  })
//   client.publish(topic, 'nodejs mqtt test', { qos: 1, retain: false }, (error) => {
//     if (error) {
//       console.error(error)
//     }
//   })
})
client.on('message', (topic, payload) => {
  //console.log('Received Message:', topic, payload.toString())
  console.log('Received Message:', [topic, payload.toString()].join(": "))

  //coba
  //console.log('Received Message:', [topic, payload.toString([0])].join(": "))

  //SQL
  // connection.query('INSERT INTO data VALUES(NULL, "'+message.toString()+'", "40", "datetime"', function (err, result) {
  //   if (err) throw err;
  //   console.log("Result: " + result);
  // });
  connection.connect(function async (err){
    
    let dataUser
    const BLEId = JSON.parse(payload.toString()).deviceFound
    
    var searchSQL = `SELECT * FROM user WHERE address = ?`
    var sql = "INSERT INTO datas_ble (nama, address) VALUES (?, ?)"
    connection.query(searchSQL, [BLEId], function(err, result) {
      if (err) throw err
      dataUser = result[0]
      connection.query(sql, [dataUser.owner, dataUser.address], function(err, result) {
        if(err) throw err
        console.log("Result: " + result)
      })
    })

  })
})
