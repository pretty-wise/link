import { Component, OnInit } from '@angular/core';
import { Observable, interval, Subscription } from 'rxjs';
import { takeWhile } from 'rxjs/operator';
import { PluginService } from '../plugin.service';
import { Plugin } from '../shared/plugin';
import { ActivatedRoute } from '@angular/router';
import { HttpClient } from '@angular/common/http';

export interface CpuStatsResponse {
  platform: string;
  error: boolean;
  timestamp: number;
  timeslice: number;
  user: number;
  system: number;
  /* on linux:
  cpu_num: number;
  ticks_per_sec: number;
  timestamp: number;
  timeslice: number;
  total: {
    user: number;
    nice: number;
    system: number;
    idle: number;
    iowait: number;
    irq: number;
    softirq: number;
  }
  process: {
    user: number;
    system: number;
  }*/
}

@Component({
  selector: 'app-monitor',
  templateUrl: './monitor.component.html',
  styleUrls: ['./monitor.component.css']
})
export class MonitorComponent implements OnInit {

  constructor(
    private pluginService: PluginService,
    private route: ActivatedRoute,
    private http: HttpClient) { }

  monitors: Observable<Plugin[]>;

  monitor: Plugin;

  CPU_STATS_URI : string = "/{0}/{1}/monitor/{2}/cpu-stats";
  max_sample_count : number = 60;

  ticker : Subscription;
  chart;

  data = {
    labels: [],
    series: [ 
      { name: "proc_user", data: [] },
      { name: "proc_system", data: [] }
    ]
  };

  startPlot() : void {
    this.chart = new Chartist.Line('.ct-chart', 
      this.data, {
        fullWidth: true,
        chartPadding: {
          right: 40
        }
      }
    );

    var hostname = this.pluginService.getHostname();
    this.ticker = interval(1000).subscribe(value => {
      console.log(value);
      var url = hostname + '/' + this.monitor.hostname + '/' + this.monitor.pid + '/' + this.monitor.name + '/' + this.monitor.version + '/cpu-stats'
      this.http.get<CpuStatsResponse>(url).subscribe((response: CpuStatsResponse) => {
        this.data.labels.push(response.timestamp);
        this.data.series[0].data.push(response.user);
        this.data.series[1].data.push(response.system);

        if(this.data.labels.length > this.max_sample_count) {
          this.data.labels.shift();
          this.data.series[0].data.shift();
          this.data.series[1].data.shift();
        }
        this.chart.update(this.data);
      });
    });
  }

  ngOnInit() {
    let index = this.route.snapshot.paramMap.get('id');
    console.log(index);
    this.monitors = this.pluginService.getMonitorList();

    this.monitors.subscribe(monitors => { 
      this.monitor = monitors[index]; 
      this.startPlot();
    });
  }

  ngOnDestroy() {
    this.ticker.unsubscribe();
  }
}
